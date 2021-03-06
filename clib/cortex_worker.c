// Some everyday C includes
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
// Hashes
#include <uthash.h>
// JSON
#include "jsoninclude.h"
// Gearman 
#include <libgearman/gearman.h>
// Self
#include "cortex_worker.h"
#include "cortex_utils.h"
// Services:
// #include<cortex/myservice.h>
#include "idf_score.h"
#include "llamapun_interface.h"

/* Usually you would define your services in separate libraries that you import via #includes */
json_object* cortex_example_service (json_object* workload) {
  json_object_object_get(workload,"document");
  printf("Hello from the example CorTeX service!\n");
  return cortex_response_json("","Example service successfully ended",-1); }
struct service_registration* cortex_services;

int main(int argc, char** argv) {
  if (argc<2) {
    printf("Missing arguments. Invoke as: \ncortex_worker [service_name] [gearman_uri] [gearman_port?] [timeout?]\n");
    return 1; }
  // Add CorTeX services here:
  // Example 1:
  struct service_registration* example_service_registration = (struct service_registration*)malloc(sizeof(struct service_registration));
  example_service_registration->name = "example_service_v0_1";
  example_service_registration->callback = cortex_example_service;
  HASH_ADD_KEYPTR( hh, cortex_services, example_service_registration->name, strlen(example_service_registration->name), example_service_registration);
  // Example 2:
  struct service_registration* idf_service_registration = (struct service_registration*)malloc(sizeof(struct service_registration));
  idf_service_registration->name = "idf_score_v0_1";
  idf_service_registration->callback = compute_idf_score;
  HASH_ADD_KEYPTR( hh, cortex_services, idf_service_registration->name, strlen(idf_service_registration->name), idf_service_registration);
  // N-Grams:
  struct service_registration* ngram_service_registration = (struct service_registration*)malloc(sizeof(struct service_registration));
  ngram_service_registration->name = "ngram_v0_1";
  ngram_service_registration->callback = get_ngrams;
  HASH_ADD_KEYPTR( hh, cortex_services, ngram_service_registration->name, strlen(ngram_service_registration->name), ngram_service_registration);
  // bag of words collection for paragraph discrimination:
  struct service_registration* bag_of_words_service_registration = (struct service_registration*)malloc(sizeof(struct service_registration));
  bag_of_words_service_registration->name = "bag_of_words_v0_1";
  bag_of_words_service_registration->callback = get_bags_of_words;
  HASH_ADD_KEYPTR( hh, cortex_services, bag_of_words_service_registration->name, strlen(bag_of_words_service_registration->name), bag_of_words_service_registration);

  // Obtain the callback for the currently requested service:
  struct service_registration* service_description;
  HASH_FIND_STR(cortex_services, argv[1], service_description);  /* word already in the hash? */
  if (service_description==NULL) { // Service not registered
      printf("Unknown name of CorTeX service, exiting!\n");
      return 1; }
  else {
    // Default Gearman parameters:
    char* gearman_uri = "127.0.0.1";
    int gearman_port = GEARMAN_DEFAULT_TCP_PORT;
    int timeout = 120;
    // Override with command line arguments:
    if ((argv[2] != NULL) && strcmp(argv[2],"")) {
      gearman_uri = strdup(argv[2]);
      if ((argv[3] != NULL) && strcmp(argv[3],"")) {
        gearman_port = atoi(argv[3]);
        if ((argv[4] != NULL) && strcmp(argv[4],"")) {
          timeout = atoi(argv[4]); } } }
    // Start a Loop with that service:
    cortex_loop(gearman_uri, gearman_port, timeout, service_description->name, service_description->callback);
  }
  return 0;
}

void cortex_loop(char* gearman_uri, int gearman_port, int timeout, char* service_name,
  json_object *(*callback)(json_object *)) {

  // Then prepare a Gearman session
  gearman_worker_st worker;
  gearman_worker_create(&worker);
  gearman_worker_add_server(&worker, gearman_uri, gearman_port);


  gearman_worker_add_function(&worker, service_name, timeout, cortex_process_document, callback);
  while(1) {
    gearman_worker_work(&worker);
  }
  gearman_worker_free(&worker);
  return; }

void *cortex_process_document(gearman_job_st *job, void *context, size_t *size, gearman_return_t *ret) {
  // Read in a workload from Gearman:
  inner_json_callback_t callback = (inner_json_callback_t) context;

  size_t workload_size = gearman_job_workload_size(job);
  char* workload = (char *)gearman_job_take_workload(job,&workload_size);
  workload = (char *)realloc(workload, workload_size+1);
  workload[workload_size] = '\0';
  json_object * json_workload = json_tokener_parse(workload);
  // Invoke callback on the workload and get the response:
  json_object* json_response = callback(json_workload);
  // Return to Gearman:
  free(workload);
  *ret=GEARMAN_SUCCESS;
  return cortex_stringify_response(json_response,size); }