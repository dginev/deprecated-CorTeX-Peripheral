# /=====================================================================\ #
# |  CorTeX Framework                                                   | #
# | Initial Preprocessing from Tex to TEI-near XHTML                    | #
# |=====================================================================| #
# | Part of the LaMaPUn project: https://trac.kwarc.info/lamapun/       | #
# |  Research software, produced as part of work done by:               | #
# |  the KWARC group at Jacobs University                               | #
# | Copyright (c) 2012                                                  | #
# | Released under the GNU Public License                               | #
# |---------------------------------------------------------------------| #
# | Deyan Ginev <d.ginev@jacobs-university.de>                  #_#     | #
# | http://kwarc.info/people/dginev                            (o o)    | #
# \=========================================================ooo==U==ooo=/ #
package CorTeX::Service::lemma_proof_spotter_v0_1;
use warnings;
use strict;
use Data::Dumper;
use XML::LibXML;
use JSON::XS;
use base qw(CorTeX::Service);

sub type {'analysis'}

sub analyze {
  my ($self,$workload) = @_;
  my $options = $workload && eval {decode_json($workload);};
  my $status = -4; # Fatal unless we succeed
  my $log; # TODO: Any messages?
 
  my $doc = $options->{document};
  my $entry = $options->{entry};

  # 2. Now, obtain a LibXML DOM of the document string
  my $parser=XML::LibXML->new();
  $parser->load_ext_dtd(0);
  my $workload_dom = $parser->parse_string($doc);
  # 3. Get an XPath context

  my $xpc = XML::LibXML::XPathContext->new($workload_dom);
  $xpc->registerNs("xhtml", "http://www.w3.org/1999/xhtml");
  my @lemmas_with_proofs = $xpc->findnodes('//xhtml:div[contains(@class,"ltx_theorem_lemma") and '
                                          .' following-sibling::xhtml:div[contains(@class,"ltx_proof")] ]');

  my $annotations = '<rdf:RDF
  xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#"
  xmlns:comsem="http://kwarc.info/comsem#">';

  foreach my $lemma(@lemmas_with_proofs) {
    my $lemma_id = $lemma->getAttribute('id');
    my $proof = $lemma->nextNonBlankSibling();
    my $proof_id = $proof->getAttribute('id');
    $annotations .=
     '<rdf:Description rdf:about='.$entry.'#'.$lemma_id.'">
        <comsem:proof rdf:resource="'.$entry.'#'.$proof_id.'"
      </rdf:Description>';
  }
  $annotations .= '</rdf:RDF>';
  return {annotations => $annotations, status=>-1};
}

1;