/* CorTeX Worker API methods for easy Gearman communication
   and JSON processing */

struct service_registration {
  char *name; /* we'll use this field as the key */
  json_object *(*callback)(json_object *);
  UT_hash_handle hh; /* makes this structure hashable */
};
// Note: Register new available CorTeX services in cortex_worker.c
extern struct service_registration *cortex_services;


typedef json_object* (*inner_json_callback_t)(json_object*);

void cortex_loop(char* gearman_uri, int gearman_port, int timeout, char* service_name,
  json_object *(*callback)(json_object *));

char* cortex_stringify_response(json_object* response, size_t *size);
json_object* cortex_response_json(char *annotations, char *message, int status);

void *cortex_process_document(gearman_job_st *job, void *context, size_t *size, gearman_return_t *ret);

json_object* cortex_example_service (json_object* workload);