use strict;
use warnings;

use Test::More tests => 1;

my $eval_return = eval {
  use CorTeX::Service;
  use CorTeX::Service::mock_spotter_v0_1;
  use CorTeX::Service::defmock_spotter_v0_1;
  1;
};

ok($eval_return && !$@, 'CorTeX::Service modules Loaded successfully.');