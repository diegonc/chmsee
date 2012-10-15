/*
 *  Copyright (C) 2011 Ji YongGang <jungleji@gmail.com>
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

#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <chm_lib.h>

#include "nsStringAPI.h"
#include "nsEmbedString.h"
#include "nsMemory.h"
#include "nsIClassInfoImpl.h"

#include "csChm.h"
#include "csChmfile.h"

csChm::csChm()
{
        /* member initializers and constructor code */
        mHomepage = (char*)nsMemory::Clone("/", 2);
        mBookname = (char*)nsMemory::Clone("", 1);
        mHhc = NULL;
        mHhk = NULL;
        mLcid = 0x0409; // default: iso-8859-1
}

csChm::~csChm()
{
        /* destructor code */
        if (mHomepage)
                nsMemory::Free(mHomepage);

        if (mBookname)
                nsMemory::Free(mBookname);

        if (mHhc)
                nsMemory::Free(mHhc);

        if (mHhk)
                nsMemory::Free(mHhk);
}

void csChm::copyinfo(char **mTarget, char *iSource)
{
        if (iSource) {
                if (*mTarget) {
                        nsMemory::Free(*mTarget);
                }

                *mTarget = (char*) nsMemory::Clone(iSource, strlen(iSource) + 1);
                free(iSource);
        }
}

NS_IMETHODIMP csChm::getAttribute(char **attr, char *m)
{
        NS_PRECONDITION(attr != nullptr, "null ptr");
        if (!attr)
                return NS_ERROR_NULL_POINTER;

        if (m) {
                *attr = (char*) nsMemory::Clone(m, strlen(m) + 1);
                if (! *attr)
                        return NS_ERROR_NULL_POINTER;
        }
        else {
                *attr = nullptr;
        }
        return NS_OK;
}

NS_IMPL_CLASSINFO(csChm, NULL, 0, CS_CHM_CID)
NS_IMPL_ISUPPORTS1_CI(csChm, csIChm)

/* long openChm (in nsILocalFile file); */
NS_IMETHODIMP csChm::OpenChm(nsILocalFile *file, const char *folder, int32_t *_retval)
{
        if (!file) {
                *_retval = -1;
                return NS_ERROR_NULL_POINTER;
        }

        // Get file native path from nsIFile interface
        nsEmbedCString path;
        file->GetNativePath(path);

        // Get filename
        char *filename = NS_CStringCloneData(path);

        struct chmFile* chmfile = chm_open(filename);
        if (!chmfile) {
                *_retval = -2;
                return NS_OK;
        }

        d(printf("csChm::OpenChm >>> Open chmfile %s\n", filename));

        long ret = extract_chm(filename, folder);
        d(printf("csChm::OpenChm >>> extract chmfile to %s, return value = %ld\n", folder, ret));

        if (ret) {
                fprintf(stderr, "extracting chm failed, file = %s\n", filename);
                return NS_ERROR_FAILURE;
        }

        struct fileinfo *info = (struct fileinfo *)malloc(sizeof(struct fileinfo));
        info->bookfolder = folder;
        info->homepage = NULL;
        info->bookname = NULL;
        info->hhc = NULL;
        info->hhk = NULL;

        chm_fileinfo(info);

        copyinfo(&mHomepage, info->homepage);
        copyinfo(&mBookname, info->bookname);
        copyinfo(&mHhc, info->hhc);
        copyinfo(&mHhk, info->hhk);

        mLcid = info->lcid;

        free(info);

        chm_close(chmfile);
        chmfile = NULL;

        return NS_OK;
}

/* readonly attribute string homepage; */
NS_IMETHODIMP csChm::GetHomepage(char **aHomepage)
{
        return getAttribute(aHomepage, mHomepage);
}

/* readonly attribute string bookname; */
NS_IMETHODIMP csChm::GetBookname(char **aBookname)
{
        return getAttribute(aBookname, mBookname);
}

/* readonly attribute string hhc; */
NS_IMETHODIMP csChm::GetHhc(char **aHhc)
{
        return getAttribute(aHhc, mHhc);
}

/* readonly attribute string hhk; */
NS_IMETHODIMP csChm::GetHhk(char **aHhk)
{
        return getAttribute(aHhk, mHhk);
}

/* readonly attribute PRUint32 lcid; */
NS_IMETHODIMP csChm::GetLcid(uint32_t *aLcid)
{
        *aLcid = mLcid;
        return NS_OK;
}
