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
package CorTeX::Service::tex_to_tei_xhtml_v0_1;
use warnings;
use strict;
use Scalar::Util qw/blessed/;
use JSON::XS qw(encode_json decode_json);
use base qw(CorTeX::Service);
use LaTeXML;
use LaTeXML::Common::Config;
use LLaMaPUn::LaTeXML;
use LLaMaPUn::Preprocessor::Purify;
use LLaMaPUn::Preprocessor::MarkTokens;

our $opts=LaTeXML::Common::Config->new(local=>1,whatsin=>'document',whatsout=>'document',
  format=>'dom',mathparse=>'no',timeout=>120,post=>0,preload=>['[ids]latexml.sty'],
  defaultresources=>0,css=>['http://latexml.mathweb.org/css/external/LaTeXML.css']);
$opts->check;

sub type {'conversion'}

sub convert {
  my ($self,$workload) = @_;
  # I. Convert to XML
  $workload = $workload && eval {decode_json($workload)};
  my $document = $workload && (ref $workload) && $workload->{document};
  return {status=>-4,log=>"Fatal:workload:empty No workload provided on input"}
    unless ($document && (length($document)>0));
  my $source = "literal:".$document;
  my $converter = LaTeXML->get_converter($opts);
  $converter->prepare_session($opts);
  my $response = $converter->convert($source);
  my ($latexml_dom, $status, $log) = map { $response->{$_} } qw(result status_code log) if defined $response;
  return {status=>-4,log=>$log."\nFatal:LaTeXML:empty No result created by LaTeXML"}
    unless (blessed($latexml_dom));
  
  # II. Purify
  # TODO: Append the Purification reports to the $log
  my $purified_dom = LLaMaPUn::Preprocessor::Purify::purify_noparse($latexml_dom,verbose=>0);
  return {status=>-4,log=>$log."\nFatal:Purify:empty Purification did not return a result."}
   unless blessed($purified_dom);
  
  # III. Tokenize
  # TODO: Append the Tokenization reports to the $log
  my $marktokens = LLaMaPUn::Preprocessor::MarkTokens->new(document=>$purified_dom,verbose=>0);
  my $tokenized_dom = $marktokens->process_document;
  return {status=>-4,log=>$log."\nFatal:MarkTokens:empty Tokenization did not return a result."}
    unless blessed($tokenized_dom);

  # IV. Move to TEI HTML
  # TODO: Append the Post-processing reports to the $log
  my $html_dom = xml_to_TEI_xhtml($tokenized_dom);
  return {status=>-4,log=>$log."\nFatal:TEI-XHTML:empty Post-processing did not return a result."}
   unless blessed($html_dom);

  return {document=>$html_dom->toString(1),
          status=>(-$status-1), # Adapt to the CorTeX scheme
          log=>$log };
}

1;