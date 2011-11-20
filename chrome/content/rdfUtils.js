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

var EXPORTED_SYMBOLS = ["RDF"];

const Cc = Components.classes;
const Ci = Components.interfaces;
const Cr = Components.results;
const Cu = Components.utils;

Cu.import("chrome://chmsee/content/utils.js");

const rdfService = Cc["@mozilla.org/rdf/rdf-service;1"].getService(Ci.nsIRDFService);
const rdfContainerUtils = Cc["@mozilla.org/rdf/container-utils;1"].createInstance(Ci.nsIRDFContainerUtils);

var RDF = {
    saveBookinfo: function (book) {
        var infoDS = rdfService.GetDataSourceBlocking("file://" + book.folder + "/chmsee_bookinfo.rdf");
        var res = rdfService.GetResource("urn:chmsee:bookinfo");
        d("RDF::saveBookinfo", "bookinfo = " + "file://" + book.folder + "/chmsee_bookinfo.rdf");

        var predicate = rdfService.GetResource("urn:chmsee:rdf#homepage");
        var object = rdfService.GetLiteral(book.homepage);
        infoDS.Assert(res, predicate, object, true);

        predicate = rdfService.GetResource("urn:chmsee:rdf#title");
        object = rdfService.GetLiteral(book.title);
        infoDS.Assert(res, predicate, object, true);

        predicate = rdfService.GetResource("urn:chmsee:rdf#charset");
        object = rdfService.GetLiteral(book.charset);
        infoDS.Assert(res, predicate, object, true);

        predicate = rdfService.GetResource("urn:chmsee:rdf#zoom");
        var oldZoom = getTargetValue(infoDS, res, "urn:chmsee:rdf#zoom") || 1.0;
        oldObject = rdfService.GetLiteral(oldZoom);
        newObject = rdfService.GetLiteral(book.zoom);
        infoDS.Change(res, predicate, oldObject, newObject);

        if (book.hhc !== null) {
            predicate = rdfService.GetResource("urn:chmsee:rdf#hhc");
            object = rdfService.GetLiteral(book.hhc);
            infoDS.Assert(res, predicate, object, true);
        }

        if (book.hhk !== null) {
            predicate = rdfService.GetResource("urn:chmsee:rdf#hhk");
            object = rdfService.GetLiteral(book.hhk);
            infoDS.Assert(res, predicate, object, true);
        }

        infoDS.QueryInterface(Ci.nsIRDFRemoteDataSource);
        infoDS.Flush();
    },

    loadBookinfo: function (book) {
        var infoFile = Cc["@mozilla.org/file/local;1"].createInstance(Ci.nsILocalFile);
        infoFile.initWithPath(book.folder + "/chmsee_bookinfo.rdf");
        d("RDF::loadBookinfo", "bookinfo = " + infoFile.path);

        if (infoFile.exists() === true) { // Reading saved bookinfo
            var infoDS = rdfService.GetDataSourceBlocking("file://" + infoFile.path);
            var res = rdfService.GetResource("urn:chmsee:bookinfo");

            book.homepage = getTargetValue(infoDS, res, "urn:chmsee:rdf#homepage") || "";
            d("RDF::loadBookinfo", "bookinfo homepage = " + book.homepage);

            book.title = getTargetValue(infoDS, res, "urn:chmsee:rdf#title") || "";
            d("RDF::loadBookinfo", "bookinfo title = " + book.title);

            book.charset = getTargetValue(infoDS, res, "urn:chmsee:rdf#charset") || "ISO-8859-1";
            d("RDF::loadBookinfo", "bookinfo charset = " + book.charset);

            book.zoom = getTargetValue(infoDS, res, "urn:chmsee:rdf#zoom") || 1.0;
            d("RDF::loadBookinfo", "bookinfo zoom = " + book.zoom);

            book.hhc = getTargetValue(infoDS, res, "urn:chmsee:rdf#hhc") || null;
            d("RDF::loadBookinfo", "bookinfo hhc = " + book.hhc);

            if (book.hhc !== null) {
                var rdf = book.hhc.slice(0, book.hhc.lastIndexOf(".hhc")) + "_hhc.rdf";
                book.hhcDS = rdfService.GetDataSourceBlocking("file://" + rdf);
            }

            book.hhk = getTargetValue(infoDS, res, "urn:chmsee:rdf#hhk") || null;
            d("RDF::loadBookinfo", "bookinfo hhk = " + book.hhk);

            if (book.hhk !== null) {
                var rdf = book.hhk.slice(0, book.hhk.lastIndexOf(".hhk")) + "_hhk.rdf";
                book.hhkDS = rdfService.GetDataSourceBlocking("file://" + rdf);
            }

            book.type = "book";
            return true;
        } else
            return false;
    },

    getRdfDatasource: function (type, book) {
        var rdfPath, treeType, path;

        if (type == "hhc") {
            rdfPath = book.hhc.slice(0, book.hhc.lastIndexOf(".hhc")) + "_hhc.rdf";
            treeType = true;
            path = book.hhc;
        } else if (type == "hhk") {
            rdfPath = book.hhk.slice(0, book.hhk.lastIndexOf(".hhk")) + "_hhk.rdf";
            treeType = false;
            path = book.hhk;
        } else
            return null;

        d("RDF::getRdfDatasource", "rdfPath = " + rdfPath);

        var datasource = rdfService.GetDataSourceBlocking("file://" + rdfPath);
        var sourceFile = Cc["@mozilla.org/file/local;1"].createInstance(Ci.nsILocalFile);
        sourceFile.initWithPath(path);

        generateRdf(treeType, sourceFile, book.folder, datasource, book.charset);

        datasource.QueryInterface(Ci.nsIRDFRemoteDataSource);
        datasource.Flush();

        return datasource;
    },

    convertDSToArray: function (datasource) {
        var data = [];
        var resource = rdfService.GetResource("urn:chmsee:root");
        var container = rdfContainerUtils.MakeSeq(datasource, resource);

        var children = container.GetElements();
        while (children.hasMoreElements()){
            var child = children.getNext();
            if (child instanceof Ci.nsIRDFResource){
                var name = getTargetValue(datasource, child, "urn:chmsee:rdf#name");
                var local = getTargetValue(datasource, child, "urn:chmsee:rdf#local");

                data.push({name: name, local: local});
            }
        }

        return data;
    },
};

var getTargetValue = function (datasource, resource, urn) {
    var target = datasource.GetTarget(resource, rdfService.GetResource(urn), true);

    if (target instanceof Ci.nsIRDFLiteral)
        return target.Value;
    else
        return null;
};

String.prototype.ncmp = function(str) {
    return this.toLowerCase() == str.toLowerCase() ? true : false;
};

var ContentHandler = function (parseInfo) {
    this.ds = parseInfo.ds;
    this.folder = parseInfo.folder;
    this.isItem = false;
    this.name = "";
    this.local = "";
    this.containers = [];
    this.lastRes = null;
    this.treeType = parseInfo.type; // false: list, true: tree
};

ContentHandler.prototype = {
    startElement: function(tag, attrs) {
        d("Handler::startElement", "tag = " + tag + ", attrs length = " + attrs.length);

        if (tag.ncmp("ul")) {
            var resource = null;

            if (this.treeType) {
                if (this.containers.length == 0) {
                    resource = rdfService.GetResource("urn:chmsee:root");
                } else {
                    resource = this.lastRes;
                }
                var container = rdfContainerUtils.MakeSeq(this.ds, resource);
                this.containers.push(container);
            } else {
                if (this.containers.length == 0) {
                    resource = rdfService.GetResource("urn:chmsee:root");
                    var container = rdfContainerUtils.MakeSeq(this.ds, resource);
                    this.containers.push(container);
                }
            }
        } else if (tag.ncmp("object")) {
            if (attrs.length > 0 && attrs[0].name.toLowerCase() == "type" && attrs[0].value.toLowerCase() == "text/sitemap") {
                this.isItem = true;
                d("Handler::startElement", "attrs[0].name = " + attrs[0].name + ", attrs[0].value = " + attrs[0].value + ", item = " + this.isItem);
            }
        } else if (tag.ncmp("param")) {
            if (attrs.length > 0 && attrs[0].value.toLowerCase() == "name")
                this.name = attrs[1].value;
            if (attrs.length > 0 && attrs[0].value.toLowerCase() == "local")
                this.local = attrs[1].value;
        }
    },

    endElement: function(tag) {
        d("Handler::endElement", "name = " + tag);
        if (tag.ncmp("ul")) {
            if (this.treeType) {
                if (this.containers.length > 0) {
                    this.containers.pop();
                }
            }
        } else if (tag.toLowerCase() == "object" && this.isItem) {
            var res = rdfService.GetAnonymousResource();

            var predicate = rdfService.GetResource("urn:chmsee:rdf#name");
            var object = rdfService.GetLiteral(this.name);
            this.ds.Assert(res, predicate, object, true);

            predicate = rdfService.GetResource("urn:chmsee:rdf#local");
            object = rdfService.GetLiteral(this.folder + "/" + this.local);
            this.ds.Assert(res, predicate, object, true);

            this.containers[this.containers.length - 1].AppendElement(res);

            this.lastRes = res;
            this.isItem = false;
        }
    },

    characters: function(str) {
        d("Handler::characters", str);
    },

    comment: function(str) {
        d("Handler::comment", str);
    },
};

var generateRdf = function (treeType, file, bookfolder, datasource, charset) {
    d("RDF::generateRdf", "charset = " + charset);
    var data = "";
    var fstream = Cc["@mozilla.org/network/file-input-stream;1"].createInstance(Ci.nsIFileInputStream);
    var cstream = Cc["@mozilla.org/intl/converter-input-stream;1"].createInstance(Ci.nsIConverterInputStream);

    fstream.init(file, -1, 0, 0);
    cstream.init(fstream, charset, 0, 0);

    let (str = {}) {
        let read = 0;
        do {
            read = cstream.readString(0xffffffff, str);
            data += str.value;
        } while (read != 0);
    }
    cstream.close();

    var tmpNameSpace = {};
    var sl = Cc["@mozilla.org/moz/jssubscript-loader;1"].createInstance(Ci.mozIJSSubScriptLoader);
    sl.loadSubScript("chrome://chmsee/content/simpleparser.js", tmpNameSpace);

    var parser = new tmpNameSpace.SimpleHtmlParser();
    var pinfo = {folder: bookfolder, ds: datasource, type: treeType};
    parser.parse(data, new ContentHandler(pinfo));
};
