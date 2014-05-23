#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>

struct stopword_element {
  char *word; /* we'll use this field as the key */
  bool stopper;             
  UT_hash_handle hh; /* makes this structure hashable */
};

struct word_count_element {
      char *word; /* we'll use this field as the key */
      int count;             
      UT_hash_handle hh; /* makes this structure hashable */
};

struct term_frequency_element {
      char *word; /* we'll use this field as the key */
      double TF;
      UT_hash_handle hh; /* makes this structure hashable */
};

extern struct stopword_element *stopwords;

int ascending_numeric_sort(struct term_frequency_element *a, struct term_frequency_element *b);

void words_from_xpath_nodes(xmlDocPtr doc, xmlNodeSetPtr nodes);

void record_word(struct word_count_element **hash, char *word);

json_object* compute_idf_score(json_object* json_workload);