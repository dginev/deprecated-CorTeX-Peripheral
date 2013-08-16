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
package CorTeX::Service::defmock_spotter_v0_1;
use warnings;
use strict;
use Data::Dumper;
use XML::LibXML;
use base qw(CorTeX::Service);

sub type {'analysis'}

sub analyze {
  my ($self,%options) = @_;
  my $status = -4; # Fatal unless we succeed
  my $log; # TODO: Any messages?
  my $document = $options{workload};
  my $entry = $options{entry};
  # Annotate with number of words in document
  my $parser=XML::LibXML->new();
  $parser->load_ext_dtd(0);
  my $workload_dom = $parser->parse_string($document);
  #Get an XPath context
  my $xpc = XML::LibXML::XPathContext->new($workload_dom);
  my @definition_nodes = $xpc->findnodes('//*[local-name()="span" and @class="ltx_definition"]');
  my $definition_count = scalar(@definition_nodes);
  my $result={};
  $result->{annotations}=<<"EOL";
<rdf:RDF
  xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#"
  xmlns:defmock="http://kwarc.info/defmock#">
  <rdf:Description rdf:about="file://$entry">
    <defmock:definitions>$definition_count</defmock:definitions>
  </rdf:Description>
</rdf:RDF>
EOL
  $status = $log ? -2 : -1; # If we reached here we succeeded
  $result->{status} = $status;
  $result->{log} = $log;  
  return $result; }

1;
