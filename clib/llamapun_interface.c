#include "llamapun_interface.h"
#include <string.h>
#include <libxml/parser.h>
#include <llamapun/llamapun_ngrams.h>

json_object* get_ngrams(json_object* workload) {
	json_object * doc = json_object_object_get(workload, "document");
	const char *xmlstring = json_object_get_string(doc);
	xmlDocPtr xmldoc = xmlParseMemory(xmlstring, strlen(xmlstring));
	json_object * answer = llamapun_get_ngrams (xmldoc);
	//free(xmlstring);
	json_object_put(doc);
	xmlFreeDoc(xmldoc);
	return answer;
}
