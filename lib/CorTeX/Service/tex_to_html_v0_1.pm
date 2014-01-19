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
package CorTeX::Service::tex_to_html_v0_1;
use warnings;
use strict;
use base qw(CorTeX::Service);
use LaTeXML;
use LaTeXML::Util::Config;
use Archive::Zip qw(:CONSTANTS :ERROR_CODES);
use IO::String;

our $opts=LaTeXML::Util::Config->new(local=>1,whatsin=>'archive',whatsout=>'archive::zip::perl',
  format=>'html5',mathparse=>'RecDescent',timeout=>120,post=>1,math_formats=>['pmml','cmml'],
  preload=>['[ids]latexml.sty'],css=>['http://latexml.mathweb.org/css/external/LaTeXML.css'],
  defaultresources=>0,inputencoding=>'iso-8859-1');
$opts->check;

sub type {'conversion'}
sub convert {
  my ($self,$workload) = @_;
  my $source = "literal:".$workload;
  my $converter = LaTeXML->get_converter($opts);
  $converter->prepare_session($opts);
  my $response = $converter->convert($source);
  my ($result, $status, $log) = map { $response->{$_} } qw(result status_code log) if defined $response;
  # Adapt status to the CorTeX scheme and write to _cortex_status.txt
  my $cortex_status = -$status -1; 
  my $payload='';
  my $content_handle = IO::String->new($payload);
  if ((!$result) || (ref($result) ne 'Archive::Zip::Archive')) {
    # Empty fatal errors should still return log messages
    $result = Archive::Zip->new(); }
  $result->addString("$cortex_status",'_cortex_status.txt');
  $result->addString("$log",'_cortex_log.txt');
  # Finally, write the ZIP to a string and return
  undef $payload unless ($result->writeToFileHandle($content_handle) == AZ_OK);
  return $payload; }

1;