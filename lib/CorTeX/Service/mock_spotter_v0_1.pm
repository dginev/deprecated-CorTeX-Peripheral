# /=====================================================================\ #
# |  CorTeX Framework                                                   | #
# | An example Analysis service for counting words and sentences        | #
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
package CorTeX::Service::mock_spotter_v0_1;
use warnings;
use strict;
use Data::Dumper;
use XML::LibXML;
use JSON::XS;
# Subclass the CorTeX::Service abstract class
# to inherit all necessary infrastructure
use base qw(CorTeX::Service);

sub type {'analysis'}

sub analyze {
  my ($self,$workload) = @_;
  my $options = $workload && eval {decode_json($workload);};
  my $status = -4; # Fatal unless we succeed
  my $log; # TODO: Any messages?
  my $document = $options && (ref $options) && $options->{document};
  return {status=>-4,log=>"Fatal:workload:empty No workload provided on input"}
    unless ($document && (length($document)>0));
  my $entry = $options->{entry};
  return {status=>-4,log=>"Fatal:entry:empty No entry provided on input"}
    unless ($entry && (length($entry)>0));
  # Prepare an XML parser
  my $parser=XML::LibXML->new();
  $parser->load_ext_dtd(0);
  # Parse the given workload
  my $workload_dom = $parser->parse_string($document);
  #Get an XPath context
  my $xpc = XML::LibXML::XPathContext->new($workload_dom);
  # Annotate with the number of tokenized words in the document
  my @word_nodes = $xpc->findnodes('//*[local-name()="span" and @class="ltx_word"]');
  my $word_count = scalar(@word_nodes);
  # Annotate with the number of tokenized sentences in the document
  my @sentence_nodes = $xpc->findnodes('//*[local-name()="span" and @class="ltx_sentence"]');
  my $sentence_count = scalar(@sentence_nodes);
  my $result={};
  # Annotations in RDF/XML
  $result->{annotations}=<<"EOL";
<rdf:RDF
  xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#"
  xmlns:mock="http://kwarc.info/mock#">
  <rdf:Description rdf:about="file://$entry">
    <mock:words>$word_count</mock:words>
    <mock:sentences>$sentence_count</mock:sentences>
  </rdf:Description>
</rdf:RDF>
EOL
  $status = $log ? -2 : -1; # If we reached here we succeeded
  $result->{status} = $status;
  $result->{log} = $log;  
  return $result; }

1;
