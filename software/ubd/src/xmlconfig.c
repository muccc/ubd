#include <stdio.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include "xmlconfig.h"

xmlDocPtr doc;
xmlNodePtr cur;
char *config;

void parseStory (xmlDocPtr doc, xmlNodePtr cur, char *keyword)
{
    xmlNodePtr p = xmlNewTextChild (cur, NULL, "keyword", "test");
    //xmlNodeAddContentLen(cur,"\n",1);
    return;
}

xmlDocPtr xml_parseConfig(void)
{
    doc = xmlParseFile(config);
    
    if (doc == NULL ) {
        fprintf(stderr,"Document not parsed successfully. \n");
        return (NULL);
    }
    
    cur = xmlDocGetRootElement(doc);
    
    if (cur == NULL) {
        fprintf(stderr,"empty document\n");
        xmlFreeDoc(doc);
        return (NULL);
    }
    
    if (xmlStrcmp(cur->name, (const xmlChar *) "ubdconfig")) {
        fprintf(stderr,"document of the wrong type, root node != ubdconfig");
        xmlFreeDoc(doc);
        return (NULL);
    }
    
    /*cur = cur->xmlChildrenNode;
    while (cur != NULL) {
        if ((!xmlStrcmp(cur->name, (const xmlChar *)"storyinfo"))){
            parseStory (doc, cur, "keyword");
        } 
        cur = cur->next;
    }
    */
}

void xml_save(void)
{
    if (doc != NULL) {
        xmlSetCompressMode(0);
        xmlSaveFormatFile (config, doc, 1);
    }
}

//read the config file into the internal data set
void xml_init(char *configfile)
{
    // try to remove all spaces from the input
    // this is a heuristif, from libxml2 and should be removed
    // but is needed to get indentation without counting the levels
    // inside the tree
    xmlKeepBlanksDefault(0);
    xmlIndentTreeOutput = 1;
    config = configfile;
    
    xml_parseConfig();
}
