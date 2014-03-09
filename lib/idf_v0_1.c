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
  json_object * json_workload = json_tokener_parse((char*)workload);
  json_object * json_document = json_object_object_get(json_workload,"document");
  char *document_string = json_object_get_string(json_document);
  int document_size = 0;
  document_size = strlen(document_string);
  // Parse it in LibXML and XPath all words:
  /* Init libxml */     
  xmlInitParser();
  xmlDocPtr doc;
  // TODO: Maybe also record entry name from the workload instead of anonymous.xml?
  doc = xmlReadMemory(document_string, document_size, "anonymous.xml", NULL, 0);
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



struct word_count_element {
      char *word; /* we'll use this field as the key */
      int count;             
      UT_hash_handle hh; /* makes this structure hashable */
};
struct TF_element {
      char *word; /* we'll use this field as the key */
      double TF;
      UT_hash_handle hh_TF; /* makes this structure hashable */
};

void record_word(struct word_count_element **hash, char *word) {
    struct word_count_element *w;
    HASH_FIND_STR(*hash, word, w);  /* word already in the hash? */
    if (w==NULL) { // New word
      w = (struct word_count_element*)malloc(sizeof(struct word_count_element));
      w->word = strdup(word);
      w->count = 1;
      HASH_ADD_KEYPTR( hh, *hash, w->word, strlen(w->word), w ); }
    else { // Already exists, just increment the counter:
      w->count++; } }

void words_from_xpath_nodes(xmlDocPtr doc, xmlNodeSetPtr nodes, FILE* output) {

  // Prepare counts hash
  struct word_count_element *w, *tmp, *word_counts = NULL;
  struct TF_element *w_TF, *TF = NULL;

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
      record_word(&word_counts, (char*)word);
      xmlFree(word);
    }
  }

  // TODO: Discard stopwords
  double max_count = 0; // double so that we force real division
  for(w=word_counts; w != NULL; w=w->hh.next) {
    int current_count = w->count;
    if (max_count < current_count) { max_count = current_count; } }
  fprintf(stderr, "\nMax count: %d\n\n", (int)max_count);

  for(w=word_counts; w != NULL; w=w->hh.next) {
    double current_tf = w->count / max_count;
    w_TF = (struct TF_element*)malloc(sizeof(struct TF_element));
    w_TF->word = strdup(w->word);
    w_TF->TF = current_tf;
    HASH_ADD_KEYPTR( hh_TF, TF, w_TF->word, strlen(w_TF->word), w_TF );
    /* free the hash table contents */
    HASH_DEL(word_counts, w);
    free(w);
  }

  for(w_TF=TF; w_TF != NULL; w_TF=w_TF->hh_TF.next) {
    fprintf(stderr,"%s : TF(%f)\n", w_TF->word, w_TF->TF); }
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