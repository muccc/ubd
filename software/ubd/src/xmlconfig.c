#include <stdio.h>
#include <mxml.h>
#include <syslog.h>

#include "groups.h"
#include "nodes.h"
#include "config.h"


mxml_node_t *tree = NULL;
#define xml_iterate(tree, node, element) for (node = mxmlFindElement(tree, tree,element, NULL, NULL, MXML_DESCEND); node != NULL; node = mxmlFindElement(node, tree, element, NULL, NULL, MXML_DESCEND))

 
void xml_print(mxml_node_t *t)
{
    mxml_node_t *node = t;
    int count = 0;
    do{
        syslog(LOG_DEBUG,"Node %d: type=", count++);
        switch(node->type){
            case MXML_TEXT:
                syslog(LOG_DEBUG,"TEXT");
            break;
            case MXML_INTEGER:
                syslog(LOG_DEBUG,"INTEGER");
            break;
            case MXML_ELEMENT:
                syslog(LOG_DEBUG,"ELEMENT");
            break;
            case MXML_OPAQUE:
                syslog(LOG_DEBUG,"OPAQUE");
            break;
            case MXML_REAL:
                syslog(LOG_DEBUG,"REAL");
            break;
            case MXML_IGNORE:
                syslog(LOG_DEBUG,"IGNORE");
            break;
            default:
                syslog(LOG_DEBUG,"ELSE");
            break;
        }
        syslog(LOG_DEBUG," value=");
        switch(node->type){
            case MXML_TEXT:
                syslog(LOG_DEBUG,"%s",node->value.text.string);
            break;
            case MXML_ELEMENT:
                syslog(LOG_DEBUG,"%s",node->value.element.name);
                int i;
                if( node->value.element.num_attrs )
                    syslog(LOG_DEBUG," Element contains attributes:");
                for(i=0; i<node->value.element.num_attrs; i++){
                    syslog(LOG_DEBUG," %s=%s", node->value.element.attrs[i].name, node->value.element.attrs[i].value);
                }
            break;
            case MXML_INTEGER:
                syslog(LOG_DEBUG,"%d",node->value.integer);
            break;
            default:
                syslog(LOG_DEBUG,"ELSE");
            break;
        }
        syslog(LOG_DEBUG,"\n");
    }while( (node = mxmlWalkNext(node, t, MXML_DESCEND)) );
}

char *xml_getAttribute(mxml_node_t *node, char *attribute)
{
    int i;
    for(i=0; i<node->value.element.num_attrs; i++){
        if( strcmp(attribute, node->value.element.attrs[i].name)== 0 )
            return node->value.element.attrs[i].value;
    }
    return NULL;
}

void xml_free(void)
{
    if( tree )
        mxmlDelete(tree);
}

void xml_load(char *filename)
{
    FILE *fp;
    fp = fopen(filename, "r");
    if( fp==NULL ){
        syslog(LOG_INFO, "no configuration file found\n");
        exit(1);
    }
    syslog(LOG_INFO,"Parsing configuration file\n");
    xml_free();
    tree = mxmlLoadFile(NULL, fp, MXML_NO_CALLBACK);
    //tree = mxmlLoadFile(NULL, fp, MXML_TEXT_CALLBACK);
    if( tree == NULL ){
        syslog(LOG_ERR,"configuration invalid\n");
        exit(1);
    }
    syslog(LOG_DEBUG,"configuration seems valid\n");
    fclose(fp);
}

void xml_parseNode(mxml_node_t *node)
{
    gchar *id = xml_getAttribute(node,"id");
    gchar *address = xml_getAttribute(node,"address");
    gchar *hostname = xml_getAttribute(node,"hostname");

    //syslog(LOG_INFO,"parsing node %s address %s\n", id, address);

    struct node *n = nodes_getFreeNode();
    strncpy(n->id, id, MAX_ID);
    n->id[MAX_ID-1] = 0;

    if( hostname != NULL && strlen(hostname) != 0 ){
        strncpy(n->avahiname, hostname, MAX_ID);
        strncpy(n->hostname, hostname, MAX_ID);
        //TODO: check buffer len
        strcat(n->hostname, ".local");
        n->hostname[MAX_ID-1] = 0;
    }

    if( address != NULL && strlen(address) != 0 ){
        n->netadr = g_inet_address_new_from_string(address);
        //g_assert(n->netadr != NULL);
    }else{
        //the address will be chosen automagically
        n->netadr = NULL;
    }

    int i = 0;
    mxml_node_t *group;
    xml_iterate(node, group, "group"){
        n->groups[i] = groups_getBusAddress(
                            xml_getAttribute(group,"name"));
        syslog(LOG_INFO,"adding group name=%s id=%d\n",
               xml_getAttribute(group,"name"), n->groups[i]);
        i++;
    }
    nodes_addNode(n);
}

void xml_parseNodes(mxml_node_t *nodes)
{
    mxml_node_t *node;
    syslog(LOG_DEBUG,"parsing nodes\n");
    xml_iterate(nodes, node, "node"){
        xml_parseNode(node);
    }
}

void xml_parseGroups(mxml_node_t *groups)
{
     mxml_node_t *group;
     syslog(LOG_DEBUG,"parsing groups\n");
     xml_iterate(groups, group, "group"){
        groups_addGroup(xml_getAttribute(group,"name"),
                        xml_getAttribute(group,"hostname"),
                        xml_getAttribute(group,"class"));
     }
}

void xml_parsebase(void)
{
    mxml_node_t *network = mxmlFindElement(
        tree, tree, "network", NULL, NULL, MXML_DESCEND);
    if( network != NULL ){
        config.interface = xml_getAttribute(
                network,"interface");
        config.base = xml_getAttribute(
                network,"base");
        config.multicastbase = xml_getAttribute(
                network,"multicastbase");
    }

    mxml_node_t *serial = mxmlFindElement(
        tree, tree, "serial", NULL, NULL, MXML_DESCEND);
    if( serial != NULL ){
        config.device = xml_getAttribute(
                        serial, "device");
        gchar *rate = xml_getAttribute(
                    serial, "rate");
        if( rate != NULL ){
            config.rate = g_ascii_strtoull(rate,NULL,10);
        }
    }

    mxml_node_t *bus = mxmlFindElement(
        tree, tree, "bus", NULL, NULL, MXML_DESCEND);
    if( bus != NULL ){
        gchar *timeout = xml_getAttribute(
                bus, "timeout");
        if( timeout != NULL ){
            config.nodetimeout = g_ascii_strtoull(timeout,NULL,10);
        }
    }

    mxml_node_t *syslog = mxmlFindElement(
        tree, tree, "syslog", NULL, NULL, MXML_DESCEND);
    if( syslog != NULL ){
        gchar *sysloglevel = xml_getAttribute(
                    syslog, "level");
        if( sysloglevel != NULL ){
            if( strcmp(sysloglevel,"INFO") == 0 ){
                config.sysloglevel = LOG_INFO;
            }else if( strcmp(sysloglevel,"WARNING") == 0 ){
                config.sysloglevel = LOG_WARNING;
            }
        }
    }
}

void xml_parsegroupsandnodes(void)
{
    mxml_node_t *groups = mxmlFindElement(
        tree, tree, "groups", NULL, NULL, MXML_DESCEND);
    xml_parseGroups(groups);

    mxml_node_t *nodes = mxmlFindElement(
        tree, tree, "nodes", NULL, NULL, MXML_DESCEND);
    xml_parseNodes(nodes);
}

void xml_init(char *filename)
{
    xml_load(filename);
    //xml_print(tree);
    xml_parsebase();
}
