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

var EXPORTED_SYMBOLS = ["Prefs", "LastUrls", "Bookmarks", "convertStrToUTF8", "d", "CsScheme", "notice", "geckoVersion"];

const Cc = Components.classes;
const Ci = Components.interfaces;
const Cr = Components.results;
const Cu = Components.utils;

const CsScheme = "file://";

/*** Read/Save preference ***/

const application = Cc["@mozilla.org/fuel/application;1"].getService(Ci.extIApplication);
const dirService = Cc["@mozilla.org/file/directory_service;1"].getService(Ci.nsIProperties);
const bmsvc = Cc["@mozilla.org/browser/nav-bookmarks-service;1"].getService(Ci.nsINavBookmarksService);
const homeDir = dirService.get("Home", Ci.nsIFile);

var Prefs = {
    get lastDir() {
        var path;
        if (application.prefs.has("chmsee.open.lastdir")) {
            path = application.prefs.get("chmsee.open.lastdir").value;
        } else {
            path = homeDir.path;
            application.prefs.setValue("chmsee.open.lastdir", path);
        }

        var dir = Cc["@mozilla.org/file/local;1"].createInstance(Ci.nsILocalFile);
        dir.initWithPath(path);
        return dir;
    },

    set lastDir(dir) {
        application.prefs.setValue("chmsee.open.lastdir", dir.path);
    },

    get bookshelf() {
        var path;
        if (application.prefs.has("chmsee.bookshelf.dir")) {
            path = application.prefs.get("chmsee.bookshelf.dir").value;
        } else {
            path = homeDir.path + "/.chmsee/bookshelf";
            application.prefs.setValue("chmsee.bookshelf.dir", path);
        }

        bookshelfDir = Cc["@mozilla.org/file/local;1"].createInstance(Ci.nsILocalFile);
        bookshelfDir.initWithPath(path);

        return bookshelfDir;
    },

    set bookshelf(dir) {
        application.prefs.setValue("chmsee.bookshelf.dir", dir.path);
    },
};

var LastUrls = {
    get reopen () {
        if (application.prefs.has("chmsee.open.lasturls")) {
            return application.prefs.get("chmsee.open.lasturls").value;
        } else
            return false;
    },

    set reopen (val) {
        application.prefs.setValue("chmsee.open.lasturls", val);
    },

    save: function (urls) {
        var data = JSON.stringify(urls);

        var profileDir = dirService.get("ProfD", Ci.nsIFile);

        d("LastUrls::save", "profile = " + profileDir.path);

        var urlsFile = Cc["@mozilla.org/file/local;1"].createInstance(Ci.nsILocalFile);
        urlsFile.initWithPath(profileDir.path + "/lastUrls.json");

        var foStream = Cc["@mozilla.org/network/file-output-stream;1"].createInstance(Ci.nsIFileOutputStream);
        foStream.init(urlsFile, -1, -1, 0);

        var converter = Cc["@mozilla.org/intl/converter-output-stream;1"].createInstance(Ci.nsIConverterOutputStream);
        converter.init(foStream, "UTF-8", 0, 0);
        converter.writeString(data);
        converter.close();
    },

    read: function () {
        var profileDir = dirService.get("ProfD", Ci.nsIFile);

        var urlsFile = Cc["@mozilla.org/file/local;1"].createInstance(Ci.nsILocalFile);
        urlsFile.initWithPath(profileDir.path + "/lastUrls.json");
        d("LastUrls::read", "urlsFile = " + urlsFile.path);

        var data = "";
        var fstream = Cc["@mozilla.org/network/file-input-stream;1"].createInstance(Ci.nsIFileInputStream);
        var cstream = Cc["@mozilla.org/intl/converter-input-stream;1"].createInstance(Ci.nsIConverterInputStream);
        fstream.init(urlsFile, -1, 0, 0);
        cstream.init(fstream, "UTF-8", 0, 0);

        let (str = {}) {
            let read = 0;
            do {
                read = cstream.readString(0xffffffff, str);
                data += str.value;
            } while (read != 0);
        }
        cstream.close();
        return data;
    },
};

var Bookmarks = {
    isBookmarked: function (address) {
        var newUrl = url(address);
        return bmsvc.isBookmarked(newUrl);
    },

    insertItem: function (address, title) {
        d("Bookmarks::insertItem", "address = " + address);
        try {
            var menuFolder = bmsvc.bookmarksMenuFolder;
            var newUrl = url(address);
            d("Bookmarks::insertItem", "newUrl.spec = " + newUrl.spec);

            bmsvc.insertBookmark(menuFolder, newUrl, bmsvc.DEFAULT_INDEX, title);

        } catch(e) {
            d("Bookmarks::insertItem", "error e.name = " + e.name + ", message = " + e.message);
        }
    },

    removeItem: function (spec) {
        var uri = url(spec);
        try {
            var id = bmsvc.getBookmarkIdsForURI(uri);
            d("Bookmarks::removeItem", "id = " + id);
            bmsvc.removeItem(id);
        } catch (e) {
            d("Bookmarks::removeItem", "e name = " + e.name + ", message = " + e.message);
        }
    },

    editItem: function (spec, title) {
        var uri = url(spec);
        try {
            var id = bmsvc.getBookmarkIdsForURI(uri);
            d("Bookmarks::editItem", "id = " + id);
            bmsvc.setItemTitle(id, title);
        } catch (e) {
            d("Bookmarks::editItem", "error = " + e);
        }
    },

    getItems: function () {
        var items = [];
        try {
            var historyService = Cc["@mozilla.org/browser/nav-history-service;1"].getService(Ci.nsINavHistoryService);
            var options = historyService.getNewQueryOptions();
            var query = historyService.getNewQuery();

            var menuFolder = bmsvc.bookmarksMenuFolder;

            query.setFolders([menuFolder], 1);

            var result = historyService.executeQuery(query, options);
            var rootNode = result.root;
            rootNode.containerOpen = true;

            for (var i = 0; i < rootNode.childCount; i ++) {
                var node = rootNode.getChild(i);
                d("Bookmarks::getItems", "item title = " + node.title + ", url = " + node.uri);
                items.push({title: node.title, uri: node.uri});
            }

            rootNode.containerOpen = false;
        } catch(e) {
            d("Bookmarks::getItems", "error e.name = " + e.name + ", message = " + e.message);
        }

        return items;
    },
};

var url = function (spec) {
    var ios = Cc["@mozilla.org/network/io-service;1"].getService(Ci.nsIIOService);
    return ios.newURI(spec, null, null);
};


var convertStrToUTF8 = function (string, charset) {
    d("convertStrToUTF8", "string = " + string + ", charset = " + charset);

    var UTF8Service = Cc["@mozilla.org/intl/utf8converterservice;1"].getService(Ci.nsIUTF8ConverterService);
    if (geckoVersion >= 15)
        return UTF8Service.convertStringToUTF8(string, charset, false, false);
    else
        return UTF8Service.convertStringToUTF8(string, charset, false);
};

/*** Debug ***/

const CsDebug = false;

var d = function (f, s) {
    if (CsDebug)
        dump(f + " >>> " + s + "\n");
};

var appInfo = Cc["@mozilla.org/xre/app-info;1"].getService(Ci.nsIXULAppInfo);
var geckoVersion = appInfo.platformVersion;

var notice = function (win, msg) {
    let prompts =  Cc["@mozilla.org/embedcomp/prompt-service;1"].getService(Ci.nsIPromptService);

    prompts.alert(win, "ChmSee 2.0", msg);
};
