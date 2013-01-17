/*
 *  Copyright (C) 2013 Ji YongGang <jungleji@gmail.com>
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

var EXPORTED_SYMBOLS = ["Book"];

const Cc = Components.classes;
const Ci = Components.interfaces;
const Cr = Components.results;
const Cu = Components.utils;

Cu.import("chrome://chmsee/content/utils.js");
Cu.import("chrome://chmsee/content/rdfUtils.js");

var Book = {
    getBookFromUrl: function(url) {
        d("Book::getBookFromUrl", "url = " + url);
        var book = newBook();
        book.url = url;

        var uri = Cc["@mozilla.org/network/io-service;1"].getService(Ci.nsIIOService).newURI(url, null, null);
        d("Book::getBookFromUrl", "scheme = " + uri.scheme);

        if (uri.scheme === "about") {
            if (uri.path === "chmsee") {
                book.title = "About ChmSee";
                book.url = "chrome://chmsee/content/about.html";
            } else if (uri.path === "config") {
                book.title = "about:config";
            }
        } else if (uri.scheme === "file") {
            d("Book::getBookFromUrl", "path = " + uri.path);
            d("Book::getBookFromUrl", "bookshelf = " + Prefs.bookshelf.path);
            var bookshelf = Prefs.bookshelf.path;

            if (uri.path.indexOf(bookshelf) !== -1) {
                var pos = uri.path.substring(bookshelf.length + 1).indexOf("/");
                book.folder = uri.path.substring(0, bookshelf.length + pos + 1);

                if (RDF.loadBookinfo(book) === false) {
                    d("Book::getBookFromUrl", "load book info failed, url = " + url);
                    return null;
                }

                if (book.hhkDS !== null) {
                    book.hhkData = RDF.convertDSToArray(book.hhkDS);
                }
            } else { // normal page
            }
        } else { // XXX HTTP?
        }

        return book;
    },

    getBookFromFile: function(file) {
        var book = newBook();

        var dirService = Cc["@mozilla.org/file/directory_service;1"].getService(Ci.nsIProperties);
        var bookshelf = dirService.get("Home", Ci.nsIFile).path + "/.chmsee/bookshelf";

        book.folder = bookshelf + "/" + md5Hash(file);

        if (RDF.loadBookinfo(book) === false) {
            try {
                var chmobj = Cc["@chmsee/cschm;1"].createInstance();
                chmobj.QueryInterface(Ci.csIChm);
                chmobj.openChm(file, book.folder);

                book.homepage = book.folder + "/" + chmobj.homepage;
                d("Book::getBookFromFile", "chm homepage = " + book.homepage);

                d("Book::getBookFromFile", "lcid = " + chmobj.lcid);
                book.charset = getCharset(chmobj.lcid);

                book.title = convertStrToUTF8(chmobj.bookname, book.charset);
                d("Book::getBookFromFile", "book title = " + book.title);

                book.type = "book";
                if (chmobj.hhc !== null) {
                    d("Book::getBookFromFile", "hhc = " + chmobj.hhc);
                    book.hhc = book.folder + "/" + chmobj.hhc;
                    book.hhcDS = RDF.getRdfDatasource("hhc", book);
                }

                if (chmobj.hhk !== null) {
                    d("Book::getBookFromFile", "hhk = " + chmobj.hhk);
                    book.hhk = book.folder + "/" +  chmobj.hhk;
                    book.hhkDS = RDF.getRdfDatasource("hhk", book);
                }
            } catch (e) {
                d("Book::getBookFromFile", "Loading @chmsee/cschm component fail: " + e.name + " -> " + e.message);
                return null;
            }

            RDF.saveBookinfo(book);
        }

        if (book.hhkDS !== null) {
            book.hhkData = RDF.convertDSToArray(book.hhkDS);
        }

        book.url = CsScheme + book.homepage;
        return book;
    },

    saveBookInfo: function (book) {
        RDF.saveBookinfo(book);
    },
};

var EmptyBook = {
    type: "page",
    folder: "",
    homepage: "",
    lastpage: "",
    url: "",
    title: "",
    zoom: 1.0,
    hhc: null,
    hhcDS: null,
    hhk: null,
    hhkDS: null,
    hhkData: null,
    charset: "ISO-8859-1",
};

var newBook = function () {
    var NewBook = function() {};
    NewBook.prototype = EmptyBook;
    return new NewBook();
};

var md5Hash = function (file) {
    var istream = Cc["@mozilla.org/network/file-input-stream;1"].createInstance(Ci.nsIFileInputStream);

    istream.init(file, 0x01, 0444, 0);

    var ch = Cc["@mozilla.org/security/hash;1"].createInstance(Ci.nsICryptoHash);
    ch.init(ch.MD5);
    ch.updateFromStream(istream, 0xffffffff);
    var hash = ch.finish(false);

    // return the two-digit hexadecimal code for a byte
    var toHexString = function (charCode) {
        return ("0" + charCode.toString(16)).slice(-2);
    }

    var s = [];
    for (var i = 0; i < hash.length; i++)
        s.push(toHexString(hash.charCodeAt(i)));
    return s.join("");
};

var getCharset = function (lcid) {
    switch(lcid) {
    case 0x0436:
    case 0x042d:
    case 0x0403:
    case 0x0406:
    case 0x0413:
    case 0x0813:
    case 0x0409:
    case 0x0809:
    case 0x0c09:
    case 0x1009:
    case 0x1409:
    case 0x1809:
    case 0x1c09:
    case 0x2009:
    case 0x2409:
    case 0x2809:
    case 0x2c09:
    case 0x3009:
    case 0x3409:
    case 0x0438:
    case 0x040b:
    case 0x040c:
    case 0x080c:
    case 0x0c0c:
    case 0x100c:
    case 0x140c:
    case 0x180c:
    case 0x0407:
    case 0x0807:
    case 0x0c07:
    case 0x1007:
    case 0x1407:
    case 0x040f:
    case 0x0421:
    case 0x0410:
    case 0x0810:
    case 0x043e:
    case 0x0414:
    case 0x0814:
    case 0x0416:
    case 0x0816:
    case 0x040a:
    case 0x080a:
    case 0x0c0a:
    case 0x100a:
    case 0x140a:
    case 0x180a:
    case 0x1c0a:
    case 0x200a:
    case 0x240a:
    case 0x280a:
    case 0x2c0a:
    case 0x300a:
    case 0x340a:
    case 0x380a:
    case 0x3c0a:
    case 0x400a:
    case 0x440a:
    case 0x480a:
    case 0x4c0a:
    case 0x500a:
    case 0x0441:
    case 0x041d:
    case 0x081d:
        return "ISO-8859-1";
    case 0x041c:
    case 0x041a:
    case 0x0405:
    case 0x040e:
    case 0x0418:
    case 0x041b:
    case 0x0424:
    case 0x081a:
        return "ISO-8859-2";
    case 0x0415:
        return "WINDOWS-1250";
    case 0x0419:
        return "WINDOWS-1251";
    case 0x0c01:
        return "WINDOWS-1256";
    case 0x0401:
    case 0x0801:
    case 0x1001:
    case 0x1401:
    case 0x1801:
    case 0x1c01:
    case 0x2001:
    case 0x2401:
    case 0x2801:
    case 0x2c01:
    case 0x3001:
    case 0x3401:
    case 0x3801:
    case 0x3c01:
    case 0x4001:
    case 0x0429:
    case 0x0420:
        return "ISO-8859-6";
    case 0x0408:
        return "ISO-8859-7";
    case 0x040d:
        return "ISO-8859-8";
    case 0x042c:
    case 0x041f:
    case 0x0443:
        return "ISO-8859-9";
    case 0x041e:
        return "ISO-8859-11";
    case 0x0425:
    case 0x0426:
    case 0x0427:
        return "ISO-8859-13";
    case 0x0411:
        return "cp932";
    case 0x0804:
    case 0x1004:
        return "gbk";
    case 0x0412:
        return "cp949";
    case 0x0404:
    case 0x0c04:
    case 0x1404:
        return "cp950";
    case 0x082c:
    case 0x0423:
    case 0x0402:
    case 0x043f:
    case 0x042f:
    case 0x0c1a:
    case 0x0444:
    case 0x0422:
    case 0x0843:
        return "cp1251";
    default:
        return "UTF-8";
    }
};
