# /=====================================================================\ #
# |  CorTeX Framework                                                   | #
# | Service template for CorTeX servies                               | #
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
package CorTeX::Service;
use strict;
use warnings;

use vars qw($VERSION);
$VERSION  = "0.1";

# Analysis, Converter and Aggregator services

sub new {
  my ($class, %options) = @_;
  bless {%options}, $class; }

sub process {
  my ($self,@arguments)=@_; 
  my $response = {};
  local $@ = undef;
  my $eval_return = eval {
    my $service_type = lc($self->type())||'';
    if ($service_type eq 'conversion') {$response = $self->convert(@arguments);}
    elsif ($service_type eq 'analysis') {$response = $self->analyze(@arguments);}
    elsif ($service_type eq 'aggregation') {$response = $self->aggregate(@arguments);}  
    1; };
  if (!$eval_return || $@) {
    $response = {
      status=>-4,
      log=>"Fatal:Service:process $@" }; }
  return $response; }

# Service API
sub type {return;}
sub convert {return;}
sub map {return;}
sub aggregate {return;}
sub job_limit {0;}

1;