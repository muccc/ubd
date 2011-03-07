#include <stdio.h>
#include <mxml.h>
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
        printf("Node %d: type=", count++);
        switch(node->type){
            case MXML_TEXT:
                printf("TEXT");
            break;
            case MXML_INTEGER:
                printf("INTEGER");
            break;
            case MXML_ELEMENT:
                printf("ELEMENT");
            break;
            case MXML_OPAQUE:
                printf("OPAQUE");
            break;
            case MXML_REAL:
                printf("REAL");
            break;
            case MXML_IGNORE:
                printf("IGNORE");
            break;
            default:
                printf("ELSE");
            break;
        }
        printf(" value=");
        switch(node->type){
            case MXML_TEXT:
                printf("%s",node->value.text.string);
            break;
            case MXML_ELEMENT:
                printf("%s",node->value.element.name);
                int i;
                if( node->value.element.num_attrs )
                    printf(" Element contains attributes:");
                for(i=0; i<node->value.element.num_attrs; i++){
                    printf(" %s=%s", node->value.element.attrs[i].name, node->value.element.attrs[i].value);
                }
            break;
            case MXML_INTEGER:
                printf("%d",node->value.integer);
            break;
            default:
                printf("ELSE");
            break;
        }
        printf("\n");
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
        printf("no configuration file found\n");
        exit(1);
    }
    printf("Parsing configuration file\n");
    xml_free();
    tree = mxmlLoadFile(NULL, fp, MXML_NO_CALLBACK);
    //tree = mxmlLoadFile(NULL, fp, MXML_TEXT_CALLBACK);
    if( tree == NULL ){
        printf("configuration invalid\n");
        exit(1);
    }
    printf("configuration seems valid\n");
    fclose(fp);
}

void xml_parseNode(mxml_node_t *node)
{
    gchar *id = xml_getAttribute(node,"id");
    gchar *address = xml_getAttribute(node,"address");

    //printf("parsing node %s address %s\n", id, address);

    struct node *n = nodes_getFreeNode();
    strncpy(n->id, id, MAX_ID);
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
        //printf("found new group name=%s id=%d\n",
        //       xml_getAttribute(group,"name"), n->groups[i]);
        i++;
    }
    nodes_addNode(n);
}

void xml_parseNodes(mxml_node_t *nodes)
{
    mxml_node_t *node;
    printf("parsing nodes\n");
    xml_iterate(nodes, node, "node"){
        xml_parseNode(node);
    }
}

void xml_parseGroups(mxml_node_t *groups)
{
     mxml_node_t *group;
     printf("parsing groups\n");
     xml_iterate(groups, group, "group"){
        groups_addGroup(xml_getAttribute(group,"name"),
                        xml_getAttribute(group,"class"));
     }
}

void xml_parsebase(void)
{
    mxml_node_t *network = mxmlFindElement(
        tree, tree, "network", NULL, NULL, MXML_DESCEND);
    config.interface = xml_getAttribute(
            network,"interface");
    config.base = xml_getAttribute(
            network,"base");
    config.multicastbase = xml_getAttribute(
            network,"multicastbase");
    mxml_node_t *serial = mxmlFindElement(
        tree, tree, "serial", NULL, NULL, MXML_DESCEND);
    config.device = xml_getAttribute(
                    serial, "device");
    gchar *rate = xml_getAttribute(
                serial, "rate");
    config.rate = g_ascii_strtoull(rate,NULL,10);

    mxml_node_t *bus = mxmlFindElement(
        tree, tree, "bus", NULL, NULL, MXML_DESCEND);

    gchar *timeout = xml_getAttribute(
                bus, "timeout");
    config.nodetimeout = g_ascii_strtoull(timeout,NULL,10);
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
