use strict;
use warnings;
use Data::Dumper;
use File::Slurp;
use CorTeX::Service::tex_to_tei_xhtml_v0_1;

use Test::More tests => 1;

my $service = CorTeX::Service::tex_to_tei_xhtml_v0_1->new();

my $alpha_convergent = read_file('t/resources/alpha-convergent.zip');
my $result = $service->process($alpha_convergent);
my $doc = delete $result->{document};

# Just a mock test for now
ok($doc);