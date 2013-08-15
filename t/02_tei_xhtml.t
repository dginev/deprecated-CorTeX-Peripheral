use strict;
use warnings;
use Data::Dumper;
use File::Slurp;
use CorTeX::Service::tex_to_tei_xhtml_v0_1;

my $service = CorTeX::Service::tex_to_tei_xhtml_v0_1->new();

my $alpha_convergent = <<'EOL';
\documentclass{article}
\begin{document}
A tricky tokenization for $\alpha$-convergent . Also, new sentence start.
\end{document}
EOL
my $result = $service->process(workload=>$alpha_convergent);
my $doc = delete $result->{document};
open OUT, ">", '/tmp/alpha.xhtml';
print OUT $doc;
close OUT;
print STDERR Dumper($result);
