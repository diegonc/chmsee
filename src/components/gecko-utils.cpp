/*
 *  Copyright (C) 2010 Ji YongGang <jungleji@gmail.com>
 *
 *  ChmSee is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.

 *  ChmSee is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.

 *  You should have received a copy of the GNU General Public License
 *  along with ChmSee; see the file COPYING.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 */

/*
 * Copyright (C) 2004 Imendio AB
 * Copyright (C) 2004 Marco Pesenti Gritti
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 */

#include "config.h"

#include <stdlib.h>

#include <gtkmozembed.h>
#include <gtkmozembed_glue.cpp>
#include <gtkmozembed_internal.h>

#include <nsCOMPtr.h>
#include <nsStringAPI.h>
#include <nsILocalFile.h>
#include <nsIPrefService.h>
#include <nsICommandManager.h>
#include <nsIDOMMouseEvent.h>
#include <nsIWebBrowserChrome.h>

// For zoom
#include <nsIDOMWindow.h>
// For copy and selectAll
#include <nsIClipboardCommands.h>
// For find
#include <nsIWebBrowserFind.h>

#include <nsServiceManagerUtils.h>
#include <nsIInterfaceRequestorUtils.h>
#include <nsComponentManagerUtils.h>

#include <nsIDocCharset.h>

#include "gecko-utils.h"
#include "utils.h"

static nsresult gecko_utils_init_prefs(void);
static gboolean util_split_font_string(const gchar *, gchar **, gint *);
static gboolean gecko_prefs_set_bool(const gchar *, gboolean);
static gboolean gecko_prefs_set_string(const gchar *, const gchar *);
static gboolean gecko_prefs_set_int(const gchar *, gint);

static gboolean
util_split_font_string(const gchar *font_name, gchar **name, gint *size)
{
        PangoFontDescription *desc;
        PangoFontMask         mask;
        gboolean              retval = FALSE;

        if (font_name == NULL)
                return FALSE;

        mask = (PangoFontMask) (PANGO_FONT_MASK_FAMILY | PANGO_FONT_MASK_SIZE);

        desc = pango_font_description_from_string(font_name);
        if (!desc)
                return FALSE;

        if ((pango_font_description_get_set_fields(desc) & mask) == mask) {
                *size = PANGO_PIXELS(pango_font_description_get_size(desc));
                *name = g_strdup(pango_font_description_get_family(desc));

                retval = TRUE;
        }

        pango_font_description_free(desc);

        return retval;
}

static gboolean
gecko_prefs_set_bool(const gchar *key, gboolean value)
{
        nsresult rv;
        nsCOMPtr<nsIPrefService> prefService(do_GetService(NS_PREFSERVICE_CONTRACTID, &rv));
        NS_ENSURE_SUCCESS (rv, FALSE);

        nsCOMPtr<nsIPrefBranch> pref;
        rv = prefService->GetBranch("", getter_AddRefs(pref));
        NS_ENSURE_SUCCESS (rv, FALSE);

        rv = pref->SetBoolPref(key, value);

        return NS_SUCCEEDED (rv) != PR_FALSE;
}

static gboolean
gecko_prefs_set_string(const gchar *key, const gchar *value)
{
        nsresult rv;
        nsCOMPtr<nsIPrefService> prefService(do_GetService(NS_PREFSERVICE_CONTRACTID, &rv));
        NS_ENSURE_SUCCESS (rv, FALSE);

        nsCOMPtr<nsIPrefBranch> pref;
        rv = prefService->GetBranch("", getter_AddRefs(pref));
        NS_ENSURE_SUCCESS (rv, FALSE);

        rv = pref->SetCharPref(key, value);

        return NS_SUCCEEDED (rv) != PR_FALSE;
}

static gboolean
gecko_prefs_set_int(const gchar *key, gint value)
{
        nsresult rv;
        nsCOMPtr<nsIPrefService> prefService(do_GetService(NS_PREFSERVICE_CONTRACTID, &rv));
        NS_ENSURE_SUCCESS (rv, FALSE);

        nsCOMPtr<nsIPrefBranch> pref;
        rv = prefService->GetBranch("", getter_AddRefs(pref));
        NS_ENSURE_SUCCESS (rv, FALSE);

        rv = pref->SetIntPref(key, value);

        return NS_SUCCEEDED (rv) != PR_FALSE;
}

static nsresult
gecko_utils_init_prefs(void)
{
        nsresult rv;

        nsCOMPtr<nsIPrefService> prefService(do_GetService(NS_PREFSERVICE_CONTRACTID, &rv));
        NS_ENSURE_SUCCESS (rv, rv);

        nsCOMPtr<nsILocalFile> file;
        rv = NS_NewNativeLocalFile(nsDependentCString(RESOURCE_FILE ("default-prefs.js")),
                                   PR_TRUE, getter_AddRefs(file));
        NS_ENSURE_SUCCESS (rv, rv);

        rv  = prefService->ReadUserPrefs(file);
        rv |= prefService->ReadUserPrefs(nsnull);

        NS_ENSURE_SUCCESS (rv, rv);

        return rv;
}

/* External functions */

extern "C" gboolean
gecko_utils_init(void)
{
        nsresult rv;

        static const GREVersionRange greVersion = {
                "1.9a", PR_TRUE,
                "2.0.*", PR_TRUE
        };

        char xpcomLocation[PATH_MAX];

        rv = GRE_GetGREPathWithProperties(&greVersion, 1, nsnull, 0,
                                          xpcomLocation, sizeof(xpcomLocation));
        if (NS_FAILED (rv)) {
                g_warning("GECKO_UTILS >>> Couldn't find a compatible GRE!\n");
                return FALSE;
        }

        // Startup the XPCOM Glue that links us up with XPCOM.
        rv = XPCOMGlueStartup(xpcomLocation);
        if (NS_FAILED (rv)) {
                g_warning("GECKO_UTILS >>> Couldn't start XPCOM!\n");
                return FALSE;
        }

        rv = GTKEmbedGlueStartup();
        if (NS_FAILED (rv)) {
                g_warning("GECKO_UTILS >>> Couldn't find GTKMozEmbed symbols!\n");
                return FALSE;
        }

        rv = GTKEmbedGlueStartupInternal();
        if (NS_FAILED (rv)) {
                g_warning("GECKO_UTILS >>> Could not startup embed glue (internal)!\n");
                return FALSE;
        }

        char *lastSlash = strrchr(xpcomLocation, '/');
        if (lastSlash)
                *lastSlash = '\0';

        gtk_moz_embed_set_path(xpcomLocation);

        /* gecko config directory, default location is ~/.config/chmsee/mozilla  */
        gchar *profile_dir = g_build_filename(g_get_user_config_dir(),
                                              PACKAGE,
                                              NULL);

        gtk_moz_embed_set_profile_path(profile_dir, "mozilla");
        g_free(profile_dir);

        gtk_moz_embed_push_startup();

        gecko_utils_init_prefs();

        g_debug("GECKO_UTILS >>> initialization finished.");
        return TRUE;
}

extern "C" void
gecko_utils_shutdown(void)
{
        gtk_moz_embed_pop_startup();
}

extern "C" gint
gecko_utils_get_mouse_event_button(gpointer event)
{
        nsIDOMMouseEvent *aMouseEvent;
        PRUint16          button;

        aMouseEvent = (nsIDOMMouseEvent *) event;

        aMouseEvent->GetButton(&button);

        return button + 1;
}

extern "C" gint
gecko_utils_get_mouse_event_modifiers(gpointer event)
{
        nsIDOMMouseEvent *aMouseEvent;
        PRBool            ctrl, alt, shift, meta;
        gint              mask;

        aMouseEvent = (nsIDOMMouseEvent *) event;

        aMouseEvent->GetCtrlKey(&ctrl);
        aMouseEvent->GetAltKey(&alt);
        aMouseEvent->GetShiftKey(&shift);
        aMouseEvent->GetMetaKey(&meta);

        mask = 0;
        if (ctrl)
                mask |= GDK_CONTROL_MASK;

        if (alt || meta)
                mask |= GDK_MOD1_MASK;

        if (shift)
                mask |= GDK_SHIFT_MASK;

        return mask;
}

extern "C" void
gecko_utils_set_font(gint type, const gchar *fontname)
{
        gchar *name;
        gint   size;

        name = NULL;

        if (!util_split_font_string(fontname, &name, &size)) {
                g_free(name);
                return;
        }

        switch (type) {
        case GECKO_PREF_FONT_VARIABLE:
                gecko_prefs_set_string("font.name.variable.x-western", name);
                gecko_prefs_set_int("font.size.variable.x-western", size);
                break;
        case GECKO_PREF_FONT_FIXED:
                gecko_prefs_set_string("font.name.fixed.x-western", name);
                gecko_prefs_set_int("font.size.fixed.x-western", size);
                break;
        }

        g_free(name);
}

extern "C" gboolean
gecko_utils_find(GtkMozEmbed *embed, const gchar *str, gboolean backward, gboolean match_case)
{
        g_debug("GECKO_UTILS >>> find string = %s", str);

        nsCOMPtr<nsIWebBrowser> webBrowser;
        gtk_moz_embed_get_nsIWebBrowser(embed, getter_AddRefs(webBrowser));

        nsCOMPtr<nsIWebBrowserFind> finder = do_GetInterface(webBrowser);
        if (finder) {
                finder->SetFindBackwards(backward);
                finder->SetMatchCase(match_case);
                finder->SetWrapFind(PR_TRUE);

                nsString sstr = NS_ConvertUTF8toUTF16(str);
                finder->SetSearchString(PromiseFlatString(sstr).get());

                PRBool rv;
                finder->FindNext(&rv);
                if (rv == PR_TRUE)
                        return TRUE;
        }

        return FALSE;
}

extern "C" void
gecko_utils_select_all(GtkMozEmbed *embed)
{
        nsCOMPtr<nsIWebBrowser> webBrowser;
        gtk_moz_embed_get_nsIWebBrowser(embed, getter_AddRefs(webBrowser));

        nsCOMPtr<nsIClipboardCommands> clipboard = do_GetInterface(webBrowser);
        g_debug("GECKO_UTILS >>> select all");

        if (!clipboard)
                g_warning("GECKO_UTILS >>> could not get ClipboardCommands Interface.");
        else
                clipboard->SelectAll();
}

extern "C" gboolean
gecko_utils_can_copy_selection(GtkMozEmbed *embed)
{
        nsCOMPtr<nsIWebBrowser> webBrowser;
        gtk_moz_embed_get_nsIWebBrowser(embed, getter_AddRefs(webBrowser));

        nsCOMPtr<nsIClipboardCommands> clipboard = do_GetInterface(webBrowser);

        if (!clipboard) {
                g_warning("GECKO_UTILS >>> could not get ClipboardCommands Interface.");
                return FALSE;
        } else {
                PRBool rv;
                clipboard->CanCopySelection(&rv);
                return rv;
        }
}

extern "C" void
gecko_utils_copy_selection(GtkMozEmbed *embed)
{
        nsCOMPtr<nsIWebBrowser> webBrowser;
        gtk_moz_embed_get_nsIWebBrowser(embed, getter_AddRefs(webBrowser));

        nsCOMPtr<nsIClipboardCommands> clipboard = do_GetInterface(webBrowser);

        if (!clipboard)
                g_warning("GECKO_UTILS >>> could not get ClipboardCommands Interface.");
        else
                clipboard->CopySelection();
}

extern "C" gfloat
gecko_utils_get_zoom(GtkMozEmbed *embed)
{
        nsCOMPtr<nsIWebBrowser>	webBrowser;
        nsCOMPtr<nsIDOMWindow>  domWindow;
        float zoom;

        gtk_moz_embed_get_nsIWebBrowser(GTK_MOZ_EMBED(embed), getter_AddRefs(webBrowser));
        webBrowser->GetContentDOMWindow(getter_AddRefs(domWindow));

        if (!domWindow) {
                g_warning("GECKO_UTILS >>> could not get DOMWindow.");
                return 1.0;
        }

        domWindow->GetTextZoom(&zoom);

        return zoom;
}

extern "C" void
gecko_utils_set_zoom(GtkMozEmbed *embed, gfloat zoom)
{
        g_debug("GECKO_UTILS >>> set zoom %f", zoom);
        nsCOMPtr<nsIWebBrowser>	webBrowser;
        nsCOMPtr<nsIDOMWindow>  domWindow;

        gtk_moz_embed_get_nsIWebBrowser(GTK_MOZ_EMBED(embed), getter_AddRefs(webBrowser));
        webBrowser->GetContentDOMWindow(getter_AddRefs(domWindow));

        if (!domWindow) {
                g_warning("GECKO_UTILS >>> Could not get DOMWindow.");
                return;
        }

        domWindow->SetTextZoom(zoom);
}

extern "C" void
gecko_utils_set_charset(GtkMozEmbed *embed, const char *charset)
{
        nsCOMPtr<nsIWebBrowser> webBrowser;
        gtk_moz_embed_get_nsIWebBrowser(embed, getter_AddRefs(webBrowser));

        nsCOMPtr<nsIDocCharset> docCharset = do_GetInterface(webBrowser);
        if (docCharset) {
                g_debug("GECKO_UTILS >>> set docCharset to %s", charset);
                g_strcmp0(charset, "Auto") ? docCharset->SetCharset(charset) : docCharset->SetCharset("");
         }
}

extern "C" char *
gecko_utils_get_charset(GtkMozEmbed *embed)
{
        nsCOMPtr<nsIWebBrowser> webBrowser;
        gtk_moz_embed_get_nsIWebBrowser(embed, getter_AddRefs(webBrowser));

        char *charset = NULL;

        nsCOMPtr<nsIDocCharset> docCharset = do_GetInterface(webBrowser);
        if (docCharset) {
                docCharset->GetCharset(&charset);
                g_debug("GECKO_UTILS >>> get docCharset : %s", charset);
         }

        return charset;
}
