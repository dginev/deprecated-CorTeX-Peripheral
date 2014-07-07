#include "llamapun_interface.h"
#include <string.h>
#include <libxml/parser.h>
#include <llamapun/llamapun_ngrams.h>

json_object* get_ngrams(json_object* workload) {
	json_object * doc = json_object_object_get(workload, "document");
	char *xmlstring = json_object_get_string(doc);
	json_object * answer = llamapun_get_ngrams (xmlParseMemory(xmlstring, strlen(xmlstring)));
	//free(xmlstring);
	json_object_put(doc);
	return answer;
}
