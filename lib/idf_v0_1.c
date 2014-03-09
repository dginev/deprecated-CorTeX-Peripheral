/* TO Setup on Ubuntu: 
  sudo apt-get install uthash-dev libxml++2.6-dev libgearman-dev libjson-c-dev
*/
// TO Compile:
// gcc idf_v0_1.c  -o idf_v0_1.out -ljson-c -lgearman -luthash `xml2-config --cflags` `xml2-config --libs`

// Some everyday C includes
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
// TODO: Do we really need assertions?
#include <assert.h>
// Hashes
#include <uthash.h>
// JSON
#include <json-c/json.h>
// Gearman 
#include <libgearman/gearman.h>

#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>

void words_from_xpath_nodes(xmlDocPtr doc, xmlNodeSetPtr nodes, FILE* output);
void *process_document(gearman_job_st *job, void *data, size_t *size, gearman_return_t *ret);
char *jsonify(char *annotations, char *message, int status, size_t *size);

void *process_document(gearman_job_st *job, void *data, size_t *size, gearman_return_t *ret)
{
  // Prepare result
  char *message=(char*)calloc(100,sizeof(char));

  // Read in a workload from Gearman:
	size_t workload_size = gearman_job_workload_size(job);
  void* workload = gearman_job_take_workload(job,&workload_size);
  
  // Parse it in LibXML and XPath all words:
  /* Init libxml */     
  xmlInitParser();
  xmlDocPtr doc;
  doc = xmlReadMemory(workload, workload_size, "anonymous.xml", NULL, 0);
  if (doc == NULL) {
      fprintf(stderr, "Failed to parse workload!\n");
      *ret=GEARMAN_WORK_FAIL;
      strcpy(message,"Fatal:LibXML:parse Failed to parse workload.");
      return jsonify("",message,-4,size); }

  xmlXPathContextPtr xpath_context; 
  xmlNodeSetPtr nodeset;
  xmlXPathObjectPtr xpath_result;
  xpath_context = xmlXPathNewContext(doc);
  if(xpath_context == NULL) {
      fprintf(stderr,"Error: unable to create new XPath context\n");
      xmlFreeDoc(doc); 
      strcpy(message,"Fatal:LibXML:XPath unable to create new XPath context\n");
      return jsonify("",message,-4,size); }

  /* Register XHTML namespace */
  xmlXPathRegisterNs(xpath_context,  BAD_CAST "xhtml", BAD_CAST "http://www.w3.org/1999/xhtml");
  xmlChar *xpath = (xmlChar*) "//xhtml:span[@class='ltx_word']";

  /* Evaluate xpath expression */
  xpath_result = xmlXPathEvalExpression(xpath, xpath_context);
  if(xpath_result == NULL) {
      fprintf(stderr,"Error: unable to evaluate xpath expression \"%s\"\n", xpath);
      xmlXPathFreeContext(xpath_context); 
      xmlFreeDoc(doc); 
      strcpy(message,"Fatal:LibXML:XPath unable to evaluate xpath expression\n");
      return jsonify("",message,-4,size); }

  /* Print results */
  words_from_xpath_nodes(doc, xpath_result->nodesetval, stdout);

  /* Cleanup */
  xmlXPathFreeObject(xpath_result);
  xmlXPathFreeContext(xpath_context); 
  xmlFreeDoc(doc); 
  /* Shutdown libxml */
  xmlCleanupParser();

  *ret=GEARMAN_SUCCESS;
  strcpy(message,"work completed");
  return jsonify("",message,-1,size); }

int main(int args,char* argv[]) {
  gearman_worker_st worker;
  gearman_worker_create(&worker);
  gearman_worker_add_server(&worker, "127.0.0.1", 4730);
  gearman_worker_add_function(&worker, "idf_v0_1", 120, process_document, NULL);

  // while(1) 
  gearman_worker_work(&worker);
  gearman_worker_free(&worker);
  return 0; }



struct word_hash_element {
      const char *word; /* we'll use this field as the key */
      int count;             
      UT_hash_handle hh; /* makes this structure hashable */
};

void record_word(struct word_hash_element *hash, const char *word) {
    struct word_hash_element *w;
    HASH_FIND_STR(hash, word, w);  /* word already in the hash? */
    if (w==NULL) {
      w = (struct word_hash_element*)malloc(sizeof(struct word_hash_element));
      w->word = word;
      w->count = 1;
      HASH_ADD_KEYPTR( hh, hash, w->word, strlen(w->word), w ); }
    else {
      // Already exists, just increment the counter:
      w->count++; } }


/**
 * print_xpath_nodes:
 * @nodes:    the nodes set.
 * @output:   the output file handle.
 *
 * Prints the @nodes content to @output.
 */
void words_from_xpath_nodes(xmlDocPtr doc, xmlNodeSetPtr nodes, FILE* output) {

  // Prepare counts hash
  struct word_hash_element *w, *tmp, *word_counts = NULL;

  xmlNodePtr cur;
  int size;
  int i;
  assert(output);
  size = (nodes) ? nodes->nodeNr : 0;
  for(i = 0; i < size; ++i) {
    assert(nodes->nodeTab[i]);
    if (nodes->nodeTab[i]->type == XML_ELEMENT_NODE) {
      cur = nodes->nodeTab[i];        
      xmlChar *word = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
      fprintf(stderr,"word: %s\n", word);
      record_word(word_counts, (const char*)word);
      xmlFree(word);
    }
  }

  /* free the hash table contents */
  HASH_ITER(hh, word_counts, w, tmp) {
    fprintf(stderr, "are we iterating?\n");
    fprintf(stderr,"word_counts{%s} = %d\n", w->word, w->count);
    HASH_DEL(word_counts, w);
    free(w);
  }
}

char* jsonify(char *annotations, char *message, int status, size_t *size) {
  json_object *response = json_object_new_object();
  json_object *json_log = json_object_new_string(message);
  json_object *json_status = json_object_new_int(status);
  json_object *json_annotations = json_object_new_string(annotations);

  json_object_object_add(response,"annotations", json_annotations);
  json_object_object_add(response,"log", json_log);
  json_object_object_add(response,"status", json_status);
  const char* string_response = json_object_to_json_string(response);
  *size = strlen(string_response);
  return string_response; }