/** 
 * XMLSec library
 *
 * Common XML Doc utility functions
 *
 * See Copyright for the status of this software.
 * 
 * Author: Aleksey Sanin <aleksey@aleksey.com>
 */
#include "globals.h"

#include <stdlib.h>
#include <string.h>
 
#include <libxml/tree.h>
#include <libxml/valid.h>
#include <libxml/xpath.h>
#include <libxml/parser.h>
#include <libxml/xpathInternals.h>
#include <libxml/parserInternals.h>

#include <xmlsec/xmlsec.h>
#include <xmlsec/xmltree.h>

typedef struct _xmlSecExtMemoryParserCtx {
    const unsigned char 	*prefix;
    size_t 			prefixSize;
    const unsigned char 	*buffer;
    size_t			bufferSize;
    const unsigned char 	*postfix;
    size_t 			postfixSize;
} xmlSecExtMemoryParserCtx, *xmlSecExtMemoryParserCtxPtr;

/* 
 * xmlSecParseFile:
 * @filename:
 *
 * Loads XML Doc from file. We need a special version because of 
 * c14n issue. The code is copied from xmlSAXParseFileWithData() function.
 */
xmlDocPtr
xmlSecParseFile(const char *filename) {
    static const char func[] ATTRIBUTE_UNUSED = "xmlSecParseFile";
    xmlDocPtr ret;
    xmlParserCtxtPtr ctxt;
    char *directory = NULL;
    
    if(filename == NULL){
#ifdef XMLSEC_DEBUG
        xmlGenericError(xmlGenericErrorContext,
	    "%s: filename is null\n", 
	    func);	
#endif
	return(NULL);	
    }


    xmlInitParser();

    ctxt = xmlCreateFileParserCtxt(filename);
    if (ctxt == NULL) {
	return(NULL);
    }

    /* todo: set directories from current doc? */    
    if ((ctxt->directory == NULL) && (directory == NULL))
        directory = xmlParserGetDirectory(filename);
    if ((ctxt->directory == NULL) && (directory != NULL))
        ctxt->directory = (char *) xmlStrdup((xmlChar *) directory);

    /* required for c14n! */
    ctxt->loadsubset = XML_DETECT_IDS | XML_COMPLETE_ATTRS; 
    ctxt->replaceEntities = 1;
    
    xmlParseDocument(ctxt);

    if(ctxt->wellFormed) { 
	ret = ctxt->myDoc;
    } else {
       ret = NULL;
       xmlFreeDoc(ctxt->myDoc);
       ctxt->myDoc = NULL;
    }
    xmlFreeParserCtxt(ctxt);    
    return(ret);
    
}

static int xmlSecExtMemoryParserRead(void * context, char * buffer, int len) {
    static const char func[] ATTRIBUTE_UNUSED = "xmlSecExtMemoryParserRead";
    xmlSecExtMemoryParserCtxPtr ctx;
    size_t size;
    
    if((context == NULL) || (buffer == NULL) || (len <= 0)){
#ifdef XMLSEC_DEBUG
        xmlGenericError(xmlGenericErrorContext,
	    "%s: context or buffer is null\n", 
	    func);	
#endif
	return(-1);	
    }
    ctx = (xmlSecExtMemoryParserCtxPtr)context;
    
    if((ctx->prefix != NULL) && (ctx->prefixSize > 0)) {
	size = (ctx->prefixSize < (size_t)len) ? ctx->prefixSize : (size_t)len; 
	memcpy(buffer, ctx->prefix, size);
	ctx->prefix += size;
	ctx->prefixSize -= size;
	return(size);
    } else if((ctx->buffer != NULL) && (ctx->bufferSize > 0)) {
	size = (ctx->bufferSize < (size_t)len) ? ctx->bufferSize : (size_t)len; 
	memcpy(buffer, ctx->buffer, size);
	ctx->buffer += size;
	ctx->bufferSize -= size;
	return(size);
    } else if((ctx->postfix != NULL) && (ctx->postfixSize > 0)) {
	size = (ctx->postfixSize < (size_t)len) ? ctx->postfixSize : (size_t)len; 
	memcpy(buffer, ctx->postfix, size);
	ctx->postfix += size;
	ctx->postfixSize -= size;
	return(size);
    }
    return(0);
}

/* 
 * xmlSecParseMemoryExt:
 *
 * Loads XML Doc from memory. We need a special version because of 
 * c14n issue. The code is copied from xmlSAXParseMemory() function.
 */
xmlDocPtr
xmlSecParseMemoryExt(const unsigned char *prefix, size_t prefixSize,
		     const unsigned char *buffer, size_t bufferSize, 
		     const unsigned char *postfix, size_t postfixSize) {
    static const char func[] ATTRIBUTE_UNUSED = "xmlSecParseMemoryExt";
    xmlSecExtMemoryParserCtx extCtx;
    xmlDocPtr ret;
    xmlParserCtxtPtr ctxt;
    
    if(buffer == NULL){
#ifdef XMLSEC_DEBUG
        xmlGenericError(xmlGenericErrorContext,
	    "%s: buffer is null\n", 
	    func);	
#endif
	return(NULL);	
    }
    extCtx.prefix = prefix;
    extCtx.prefixSize = prefixSize;
    extCtx.buffer = buffer;
    extCtx.bufferSize = bufferSize;
    extCtx.postfix = postfix;
    extCtx.postfixSize = postfixSize;
        
    
    ctxt = xmlCreateIOParserCtxt(NULL, NULL, xmlSecExtMemoryParserRead, 
				 NULL, &extCtx, XML_CHAR_ENCODING_NONE);
    if (ctxt == NULL) {
#ifdef XMLSEC_DEBUG
        xmlGenericError(xmlGenericErrorContext,
	    "%s: failed to create parser\n", 
	    func);	
#endif
	return(NULL);
    }

    /* required for c14n! */
    ctxt->loadsubset = XML_DETECT_IDS | XML_COMPLETE_ATTRS; 
    ctxt->replaceEntities = 1;

    xmlParseDocument(ctxt);
    ret = ctxt->myDoc; 
    xmlFreeParserCtxt(ctxt);    
    return(ret);
}


/* 
 * xmlSecParseMemory:
 * @filename:
 *
 * Loads XML Doc from memory. We need a special version because of 
 * c14n issue. The code is copied from xmlSAXParseMemory() function.
 */
xmlDocPtr
xmlSecParseMemory(const unsigned char *buffer, size_t size, int recovery) {
    static const char func[] ATTRIBUTE_UNUSED = "xmlSecParseMemory";
    xmlDocPtr ret;
    xmlParserCtxtPtr ctxt;
    
    if(buffer == NULL){
#ifdef XMLSEC_DEBUG
        xmlGenericError(xmlGenericErrorContext,
	    "%s: buffer is null\n", 
	    func);	
#endif
	return(NULL);	
    }
    
    ctxt = xmlCreateMemoryParserCtxt((char*)buffer, size);
    if (ctxt == NULL) {
#ifdef XMLSEC_DEBUG
        xmlGenericError(xmlGenericErrorContext,
	    "%s: failed to create parser\n", 
	    func);	
#endif
	return(NULL);
    }

    /* required for c14n! */
    ctxt->loadsubset = XML_DETECT_IDS | XML_COMPLETE_ATTRS; 
    ctxt->replaceEntities = 1;

    xmlParseDocument(ctxt);

    if((ctxt->wellFormed) || recovery) {
	ret = ctxt->myDoc; 
    } else {
       ret = NULL;
       xmlFreeDoc(ctxt->myDoc);
       ctxt->myDoc = NULL;
    }
    xmlFreeParserCtxt(ctxt);    
    return(ret);
}

/**
 * xmlSecFindChild:
 * @parent:
 * @name:
 * @ns:
 *
 * Lookups a direct child of the node having given name and namespace href
 * 
 */
xmlNodePtr
xmlSecFindChild(const xmlNodePtr parent, const xmlChar *name, const xmlChar *ns) {
    static const char func[] ATTRIBUTE_UNUSED = "xmlSecFindChild";
    xmlNodePtr cur;
        
    if((parent == NULL) || (name == NULL)){
#ifdef XMLSEC_DEBUG
        xmlGenericError(xmlGenericErrorContext,
	    "%s: the node or name is null\n", 
	    func);	
#endif
	return(NULL);	
    }
    
    cur = parent->children;
    while(cur != NULL) {
        if(cur->type == XML_ELEMENT_NODE) {
	    if(xmlSecCheckNodeName(cur, name, ns)) {
		return(cur);
	    }
	}
	cur = cur->next;
    }
    return(NULL);
}

xmlNodePtr
xmlSecFindParent(const xmlNodePtr cur, const xmlChar *name, const xmlChar *ns) {
    static const char func[] ATTRIBUTE_UNUSED = "xmlSecFindParent";
        
    if((cur == NULL) || (name == NULL)){
#ifdef XMLSEC_DEBUG
        xmlGenericError(xmlGenericErrorContext,
	    "%s: the node or name is null\n", 
	    func);	
#endif
	return(NULL);	
    }

    if(xmlSecCheckNodeName(cur, name, ns)) {
	return(cur);
    } else if(cur->parent != NULL) {
	return(xmlSecFindParent(cur->parent, name, ns));
    }
    return(NULL);
}

xmlNodePtr		
xmlSecFindNode(const xmlNodePtr parent, const xmlChar *name, const xmlChar *ns) {
    static const char func[] ATTRIBUTE_UNUSED = "xmlSecFindChild";
    xmlNodePtr cur;
    xmlNodePtr ret;
        
    if(name == NULL){
#ifdef XMLSEC_DEBUG
        xmlGenericError(xmlGenericErrorContext,
	    "%s: the name is null\n", 
	    func);	
#endif
	return(NULL);	
    }
    
    cur = parent;
    while(cur != NULL) {
        if((cur->type == XML_ELEMENT_NODE) && xmlSecCheckNodeName(cur, name, ns)) {
	    return(cur);
	}
	if(cur->children != NULL) {
	    ret = xmlSecFindNode(cur->children, name, ns);
	    if(ret != NULL) {
	        return(ret);	    
	    }
	}
	cur = cur->next;
    }
    return(NULL);
}




/** 
 * xmlSecCheckNodeName:
 * @cur:
dis * @name:
 * @ns:
 *
 * Checks that the node has a given name and a given namespace href
 */
int
xmlSecCheckNodeName(const xmlNodePtr cur, const xmlChar *name, const xmlChar *ns) {
    static const char func[] ATTRIBUTE_UNUSED = "xmlSecCheckNodeName";
    
    if((cur != NULL) && xmlStrEqual(cur->name, name)) {
	if(cur->ns == NULL && ns == NULL) {
	    return(1);
	} else if(cur->ns == NULL) {
	    xmlNsPtr tmp;

	    tmp = xmlSearchNs(cur->doc, cur, NULL);
	    if(tmp != NULL && xmlStrEqual(tmp->href, ns)) {
		return(1);
	    }
	} else if(xmlStrEqual(cur->ns->href, ns)){
	    return(1);	
	}
    } 
    return(0);   
}

/**
 * xmlSecAddChild
 * @parent:
 * @name:
 * @ns:
 *
 */
xmlNodePtr		
xmlSecAddChild(xmlNodePtr parent, const xmlChar *name, const xmlChar *ns) {
    static const char func[] ATTRIBUTE_UNUSED = "xmlSecAddChild";
    xmlNodePtr cur;
    xmlNodePtr text;
        
    if((parent == NULL) || (name == NULL)) {
#ifdef XMLSEC_DEBUG
        xmlGenericError(xmlGenericErrorContext,
	    "%s: parent node or name is null\n",
	    func);	
#endif
	return(NULL);
    }

    if(parent->children == NULL) {
        /* TODO: add indents */
	text = xmlNewText(BAD_CAST "\n"); 
        if(text == NULL) {	
#ifdef XMLSEC_DEBUG
    	    xmlGenericError(xmlGenericErrorContext,
		"%s: failed to create line break text node\n",
	        func);	
#endif
	    return(NULL);
	}
	xmlAddChild(parent, text);
    }

    cur = xmlNewChild(parent, NULL, name, NULL);
    if(cur == NULL) {
#ifdef XMLSEC_DEBUG
        xmlGenericError(xmlGenericErrorContext,
	    "%s: failed to create node\n",
	    func);	
#endif
	return(NULL);
    }

    /* namespaces support */
    if(ns != NULL) {
	xmlNsPtr nsPtr;
	
	nsPtr = xmlSearchNs(cur->doc, cur, NULL);
	if((nsPtr == NULL) || !xmlStrEqual(nsPtr->href, ns)) {
	    nsPtr = xmlNewNs(cur, ns, NULL);
	}
    }
    
    /* TODO: add indents */
    text = xmlNewText(BAD_CAST "\n"); 
    if(text == NULL) {	
#ifdef XMLSEC_DEBUG
        xmlGenericError(xmlGenericErrorContext,
	    "%s: failed to create line break text node\n",
	    func);	
#endif
	return(NULL);
    }
    xmlAddChild(parent, text);

    return(cur);
}

/**
 * xmlSecAddNextSibling
 * @parent:
 * @name:
 * @ns:
 *
 */
xmlNodePtr
xmlSecAddNextSibling(xmlNodePtr node, const xmlChar *name, const xmlChar *ns) {
    static const char func[] ATTRIBUTE_UNUSED = "xmlSecAddNextSibling";
    xmlNodePtr cur;
    xmlNodePtr text;
    
    if((node == NULL) || (name == NULL)) {
#ifdef XMLSEC_DEBUG
        xmlGenericError(xmlGenericErrorContext,
	    "%s: sibling node or name is null\n",
	    func);	
#endif
	return(NULL);
    }

    cur = xmlNewNode(NULL, name);
    if(cur == NULL) {
#ifdef XMLSEC_DEBUG
        xmlGenericError(xmlGenericErrorContext,
	    "%s: failed to create node\n",
	    func);	
#endif
	return(NULL);
    }
    xmlAddNextSibling(node, cur);

    /* namespaces support */
    if(ns != NULL) {
	xmlNsPtr nsPtr;
	
	nsPtr = xmlSearchNs(cur->doc, cur, NULL);
	if((nsPtr == NULL) || !xmlStrEqual(nsPtr->href, ns)) {
	    nsPtr = xmlNewNs(cur, ns, NULL);
	}
    }

    /* TODO: add indents */
    text = xmlNewText(BAD_CAST "\n");
    if(text == NULL) {	
#ifdef XMLSEC_DEBUG
        xmlGenericError(xmlGenericErrorContext,
	    "%s: failed to create line break text node\n",
	    func);	
#endif
	return(NULL);
    }
    xmlAddNextSibling(node, text);
    
    return(cur);
}

/**
 * xmlSecAddPrevSibling
 * @parent:
 * @name:
 * @ns:
 *
 */
xmlNodePtr
xmlSecAddPrevSibling(xmlNodePtr node, const xmlChar *name, const xmlChar *ns) {
    static const char func[] ATTRIBUTE_UNUSED = "xmlSecAddPrevSibling";
    xmlNodePtr cur;
    xmlNodePtr text;
    
    if((node == NULL) || (name == NULL)) {
#ifdef XMLSEC_DEBUG
        xmlGenericError(xmlGenericErrorContext,
	    "%s: sibling node or name is null\n",
	    func);	
#endif
	return(NULL);
    }

    cur = xmlNewNode(NULL, name);
    if(cur == NULL) {
#ifdef XMLSEC_DEBUG
        xmlGenericError(xmlGenericErrorContext,
	    "%s: failed to create node\n",
	    func);	
#endif
	return(NULL);
    }
    xmlAddPrevSibling(node, cur);

    /* namespaces support */
    if(ns != NULL) {
	xmlNsPtr nsPtr;
	
	nsPtr = xmlSearchNs(cur->doc, cur, NULL);
	if((nsPtr == NULL) || !xmlStrEqual(nsPtr->href, ns)) {
	    nsPtr = xmlNewNs(cur, ns, NULL);
	}
    }

    /* TODO: add indents */
    text = xmlNewText(BAD_CAST "\n");
    if(text == NULL) {	
#ifdef XMLSEC_DEBUG
        xmlGenericError(xmlGenericErrorContext,
	    "%s: failed to create line break text node\n",
	    func);	
#endif
	return(NULL);
    }
    xmlAddPrevSibling(node, text);

    return(cur);
}

/**
 * xmlSecGetNextElementNode
 * @cur:
 *
 *
 */
xmlNodePtr
xmlSecGetNextElementNode(xmlNodePtr cur) {
    
    while((cur != NULL) && (cur->type != XML_ELEMENT_NODE)) {
	cur = cur->next;
    }
    return(cur);
}

/**
 * xmlSecReplaceNode
 *
 *
 *
 *
 */
int
xmlSecReplaceNode(xmlNodePtr node, xmlNodePtr newNode) {
    static const char func[] ATTRIBUTE_UNUSED = "xmlSecReplaceNode";
    xmlNodePtr old;
    xmlNodePtr ptr;
    xmlNodePtr dummy;
    
    if((node == NULL) || (newNode == NULL)){
#ifdef XMLSEC_DEBUG
        xmlGenericError(xmlGenericErrorContext,
	    "%s: node or new node is null\n", 
	    func);	
#endif
	return(-1);	    
    }    
	    
    dummy = xmlNewDocNode(newNode->doc, NULL, BAD_CAST "dummy", NULL);
    if(dummy == NULL) {
#ifdef XMLSEC_DEBUG
    	xmlGenericError(xmlGenericErrorContext,
	    "%s: dummy node creation failed\n", 
	    func);	
#endif
	return(-1);
    }
	    
    if(newNode == xmlDocGetRootElement(newNode->doc)) {
	ptr = xmlDocSetRootElement(newNode->doc, dummy);
    } else {
	ptr = xmlReplaceNode(newNode, dummy);
    }
    if(ptr == NULL) {
#ifdef XMLSEC_DEBUG
    	xmlGenericError(xmlGenericErrorContext,
	    "%s: failed to replace template node\n", 
	    func);	
#endif
	xmlFreeNode(dummy);
	return(-1);
    }
	    
    if(node == xmlDocGetRootElement(node->doc)) {
	old = xmlDocSetRootElement(node->doc, ptr);		
    } else {
	old = xmlReplaceNode(node, ptr);
    }
    if(old == NULL) {
#ifdef XMLSEC_DEBUG
	xmlGenericError(xmlGenericErrorContext,
	    "%s: failed to replace node\n", 
	    func);	
#endif
	xmlFreeNode(ptr);
	return(-1);
    }
    xmlFreeNode(old);
    return(0);
}

/**
 * xmlSecReplaceContent
 *
 *
 *
 *
 */
int
xmlSecReplaceContent(xmlNodePtr node, xmlNodePtr newNode) {
    static const char func[] ATTRIBUTE_UNUSED = "xmlSecReplaceContent";
    xmlNodePtr dummy;
    xmlNodePtr ptr;

    if((node == NULL) || (newNode == NULL)){
#ifdef XMLSEC_DEBUG
        xmlGenericError(xmlGenericErrorContext,
	    "%s: node or new node is null\n", 
	    func);	
#endif
	return(-1);	    
    }    
	    
    dummy = xmlNewDocNode(newNode->doc, NULL, BAD_CAST "dummy", NULL);
    if(dummy == NULL) {
#ifdef XMLSEC_DEBUG
    	xmlGenericError(xmlGenericErrorContext,
	    "%s: dummy node creation failed\n", 
	    func);	
#endif
	return(-1);
    }
	    
    if(newNode == xmlDocGetRootElement(newNode->doc)) {
	ptr = xmlDocSetRootElement(newNode->doc, dummy);
    } else {
	ptr = xmlReplaceNode(newNode, dummy);
    }
    if(ptr == NULL) {
#ifdef XMLSEC_DEBUG
	xmlGenericError(xmlGenericErrorContext,
	    "%s: failed to replace template node\n", 
	    func);	
#endif
	xmlFreeNode(dummy);
	return(-1);
    }
	    
    xmlNodeSetContent(node, NULL);
    xmlAddChild(node, ptr);
    return(0);
}


/**
 * xmlSecReplaceNodeBuffer
 *
 *
 *
 *
 */
int
xmlSecReplaceNodeBuffer(xmlNodePtr node, const unsigned char *buffer, size_t size) {
    static const char func[] ATTRIBUTE_UNUSED = "xmlSecReplaceNodeBuffer";
    static const char dummyPrefix[] = "<dummy>";
    static const char dummyPostfix[] = "</dummy>";
    xmlDocPtr doc;
    xmlNodePtr ptr1, ptr2;
    
    if((node == NULL) || (buffer == NULL)){
#ifdef XMLSEC_DEBUG
        xmlGenericError(xmlGenericErrorContext,
	    "%s: node is null\n", 
	    func);	
#endif
	return(-1);	    
    }

	    
    doc = xmlSecParseMemoryExt((unsigned char*)dummyPrefix, strlen(dummyPrefix),
			       buffer, size,
			       (unsigned char*)dummyPostfix, strlen(dummyPostfix));
    if(doc == NULL){
#ifdef XMLSEC_DEBUG
    	xmlGenericError(xmlGenericErrorContext,
	    "%s: failed to parse buffer\n", 
	    func);	
#endif
	return(-1);	    	
    }
	    
    ptr1 = xmlDocGetRootElement(doc);
    if(ptr1 == NULL){
#ifdef XMLSEC_DEBUG
    	xmlGenericError(xmlGenericErrorContext,
	    "%s: doc root is null\n", 
	    func);	
#endif
	xmlFreeDoc(doc);
	return(-1);	    	
    }
    
    ptr1 = ptr1->children;
    while(ptr1 != NULL) {
	ptr2 = ptr1->next;
	xmlUnlinkNode(ptr1);
	xmlAddPrevSibling(node, ptr1);
	ptr1 = ptr2;
    }
	    
    xmlUnlinkNode(node);
    xmlFreeNode(node);  
    xmlFreeDoc(doc);
    return(0);
}

void	
xmlSecAddIDs(xmlDocPtr doc, xmlNodePtr cur, const xmlChar** ids) {
    static const char func[] ATTRIBUTE_UNUSED = "xmlSecAddIDs";
    xmlNodePtr children = NULL;
    
    if((doc == NULL) || (ids == NULL)){
#ifdef XMLSEC_DEBUG
        xmlGenericError(xmlGenericErrorContext,
	    "%s: doc or ids list is null\n", 
	    func);	
#endif
	return;	    
    }
    
    if((cur != NULL) && (cur->type == XML_ELEMENT_NODE)) {
	xmlAttrPtr attr;
	xmlAttrPtr tmp;
	int i;
	xmlChar* name;
	
	for(attr = cur->properties; attr != NULL; attr = attr->next) {
	    for(i = 0; ids[i] != NULL; ++i) {
		if(xmlStrEqual(attr->name, ids[i])) {
		    name = xmlNodeListGetString(doc, attr->children, 1);
		    if(name != NULL) {
			tmp = xmlGetID(doc, name);
			if(tmp == NULL) {
			    xmlAddID(NULL, doc, name, attr);
			} else if(tmp != attr) {
#ifdef XMLSEC_DEBUG
		    	    xmlGenericError(xmlGenericErrorContext,
				"%s: id \"%s\" already defined\n", 
				func, name);	
#endif
			}
			xmlFree(name);
		    }		    
		}
	    }
	}
	
	children = cur->children;
    } else if(cur == NULL) {
	children = doc->children;
    }
    
    while(children != NULL) {
	if(children->type == XML_ELEMENT_NODE) {
	    xmlSecAddIDs(doc, children, ids);
	}
	children = children->next;
    }
}

