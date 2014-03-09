// TO Compile:
// gcc idf_v0_1.c  -o idf_v0_1.out -lgearman -L/usr/lib/x86_64-linux_gnu/ `xml2-config --cflags` `xml2-config --libs`
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include <libgearman/gearman.h>

#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>

void words_from_xpath_nodes(xmlDocPtr doc, xmlNodeSetPtr nodes, FILE* output);
void *process_document(gearman_job_st *job, void *data, size_t *size, gearman_return_t *ret);

void *process_document(gearman_job_st *job, void *data, size_t *size, gearman_return_t *ret)
{
  // Prepare result
  char *result=(char*)calloc(50,sizeof(char));
  *size=50;

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
      strcpy(result,"Fatal:LibXML:parse Failed to parse workload.");
      return result; }

  xmlXPathContextPtr xpath_context; 
  xmlNodeSetPtr nodeset;
  xmlXPathObjectPtr xpath_result;
  xpath_context = xmlXPathNewContext(doc);
  if(xpath_context == NULL) {
      fprintf(stderr,"Error: unable to create new XPath context\n");
      xmlFreeDoc(doc); 
      strcpy(result,"Fatal:LibXML:XPath unable to create new XPath context\n");
      return result; }

  /* Register XHTML namespace */
  xmlXPathRegisterNs(xpath_context,  BAD_CAST "xhtml", BAD_CAST "http://www.w3.org/1999/xhtml");
  xmlChar *xpath = (xmlChar*) "//xhtml:span[@class='ltx_word']";

  /* Evaluate xpath expression */
  xpath_result = xmlXPathEvalExpression(xpath, xpath_context);
  if(xpath_result == NULL) {
      fprintf(stderr,"Error: unable to evaluate xpath expression \"%s\"\n", xpath);
      xmlXPathFreeContext(xpath_context); 
      xmlFreeDoc(doc); 
      strcpy(result,"Fatal:LibXML:XPath unable to evaluate xpath expression\n");
      return result; }

  /* Print results */
  words_from_xpath_nodes(doc, xpath_result->nodesetval, stdout);

  /* Cleanup */
  xmlXPathFreeObject(xpath_result);
  xmlXPathFreeContext(xpath_context); 
  xmlFreeDoc(doc); 
  /* Shutdown libxml */
  xmlCleanupParser();

  *ret=GEARMAN_SUCCESS;
  strcpy(result,"work completed");
  return result; }

int main(int args,char* argv[]) {
  gearman_worker_st worker;
  gearman_worker_create(&worker);
  gearman_worker_add_server(&worker, "127.0.0.1", 4730);
  gearman_worker_add_function(&worker, "idf_v0_1", 120, process_document, NULL);

  // while(1) 
  gearman_worker_work(&worker);
  gearman_worker_free(&worker);
  return 0; }



/**
 * print_xpath_nodes:
 * @nodes:    the nodes set.
 * @output:   the output file handle.
 *
 * Prints the @nodes content to @output.
 */
void words_from_xpath_nodes(xmlDocPtr doc, xmlNodeSetPtr nodes, FILE* output) {
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
      fprintf(stderr, "word: %s\n", word );
      xmlFree(word);
    }
  }
}
