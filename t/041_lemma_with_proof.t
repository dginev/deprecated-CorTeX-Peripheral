use strict;
use warnings;
use Data::Dumper;
use File::Slurp;
use JSON::XS qw(encode_json decode_json);

use CorTeX::Service::lemma_proof_spotter_v0_1;

use Test::More tests => 1;

my $service = CorTeX::Service::lemma_proof_spotter_v0_1->new();

my $train_doc = read_file('t/resources/ComSem-train/1311.0043/_cortex_tex_to_tei_xhtml_v0_1/1311.0043.xhtml');
my $workload = encode_json({document => $train_doc, entry=>'1311.0043'});
my $response = $service->process($workload);

#print STDERR Dumper($response);

# Just a mock test for now
ok($response->{annotations});