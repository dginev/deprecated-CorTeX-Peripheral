// Some everyday C includes
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
// Hashes
#include <uthash.h>
// JSON
#include <json-c/json.h>
// XML DOM and XPath
#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>
// Self
#include "idf_score.h"
#include "cortex_utils.h"


json_object* compute_idf_score(json_object* json_workload) {
  json_object * json_document = json_object_object_get(json_workload,"document");
  const char *document_string = json_object_get_string(json_document);
  char* log_message="";
  int document_size = 0;
  if (document_string != NULL) { document_size = strlen(document_string); }
  // Parse it in LibXML and XPath all words:
  /* Init libxml */
  xmlInitParser();
  xmlDocPtr doc;
  // TODO: Maybe also record entry name from the workload instead of anonymous.xml?
  doc = xmlReadMemory(document_string, document_size, "anonymous.xml", NULL, 0);
  if (doc == NULL) {
    fprintf(stderr, "Failed to parse workload!\n");
    log_message = "Fatal:LibXML:parse Failed to parse workload.";
    return cortex_response_json("",log_message,-4); }
  xmlXPathContextPtr xpath_context;
  xmlXPathObjectPtr xpath_result;
  xpath_context = xmlXPathNewContext(doc);
  if(xpath_context == NULL) {
    fprintf(stderr,"Error: unable to create new XPath context\n");
    xmlFreeDoc(doc);
    log_message = "Fatal:LibXML:XPath unable to create new XPath context\n";
    return cortex_response_json("",log_message,-4); }

  /* Register XHTML namespace */
  xmlXPathRegisterNs(xpath_context,  BAD_CAST "xhtml", BAD_CAST "http://www.w3.org/1999/xhtml");
  xmlChar *xpath = (xmlChar*) "//xhtml:span[@class='ltx_word']";

  /* Evaluate xpath expression */
  xpath_result = xmlXPathEvalExpression(xpath, xpath_context);
  if(xpath_result == NULL) {
    fprintf(stderr,"Error: unable to evaluate xpath expression \"%s\"\n", xpath);
    xmlXPathFreeContext(xpath_context);
    xmlFreeDoc(doc);
    log_message = "Fatal:LibXML:XPath unable to evaluate xpath expression\n";
    return cortex_response_json("",log_message,-4); }

  /* Print results */
  words_from_xpath_nodes(doc, xpath_result->nodesetval);

  /* Cleanup */
  xmlXPathFreeObject(xpath_result);
  xmlXPathFreeContext(xpath_context);
  xmlFreeDoc(doc);
  /* Shutdown libxml */
  xmlCleanupParser();
  return cortex_response_json("",log_message,-1); //TODO
}


/* Preliminary Utilities */

struct stopword_element* json_stopwords_get() {
  /* Read in the list of stopwords as a hash */
  json_object *stopwords_json = json_object_from_file("resources/stopwords.json");
  
  int stopwords_count = json_object_array_length(stopwords_json);
  struct stopword_element *stopword_hash;
  int i;
  for (i=0; i< stopwords_count; i++){
    json_object *word_json = json_object_array_get_idx(stopwords_json, i); /*Getting the array element at position i*/
    const char *stopword = json_object_get_string(word_json);
    stopword_hash = (struct stopword_element*)malloc(sizeof(struct stopword_element));
    stopword_hash->word = strdup(stopword);
    stopword_hash->stopper = true;
    HASH_ADD_KEYPTR( hh, stopwords, stopword_hash->word, strlen(stopword_hash->word), stopword_hash ); }

  return stopword_hash;
}

struct stopword_element *stopwords;

int ascending_numeric_sort(struct term_frequency_element *a, struct term_frequency_element *b) {
  double a_TF = a->TF;
  double b_TF = b->TF;
  if (a_TF < b_TF)  {return (int) 1; }
  else if (a_TF > b_TF)  {return (int) -1;  }
  else { return (int) 0;  }
}

/* Core analysis algorithm: */
void words_from_xpath_nodes(xmlDocPtr doc, xmlNodeSetPtr nodes) {

  // Prepare counts hash
  struct word_count_element *w, *word_counts = NULL;
  struct term_frequency_element *w_TF, *TF = NULL;

  xmlNodePtr cur;
  int size;
  int i;
  size = (nodes) ? nodes->nodeNr : 0;
  for(i = 0; i < size; ++i) {
    cur = nodes->nodeTab[i];
    if (cur == NULL) {continue;}
    if (nodes->nodeTab[i]->type == XML_ELEMENT_NODE) {
      xmlChar *word = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
      if (word == NULL) {continue;}
      xmlChar *p;
      for(p = word;*p;++p) *p=tolower(*p); /* Normalization: lowercase the ASCII letters */
      HASH_FIND_STR(stopwords, (char*) word, w);  /* word already in the hash? */
      if (w==NULL) { // Skip stop words
        record_word(&word_counts, (char*)word); }
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
    w_TF = (struct term_frequency_element*)malloc(sizeof(struct term_frequency_element));
    w_TF->word = strdup(w->word);
    w_TF->TF = current_tf;
    HASH_ADD_KEYPTR( hh, TF, w_TF->word, strlen(w_TF->word), w_TF );
    /* free the hash table contents */
    HASH_DEL(word_counts, w);
    free(w);
  }

  HASH_SORT(TF, ascending_numeric_sort);
  for(w_TF=TF; w_TF != NULL; w_TF=w_TF->hh.next) {
    fprintf(stderr,"%s : TF(%f)\n", w_TF->word, w_TF->TF); }
}

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
