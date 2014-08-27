#======================================================================
# Makefile Maker for CorTeX-Peripherals
# d.ginev@jacobs-university.de
#======================================================================
use ExtUtils::MakeMaker;
use strict;
use FindBin;

#======================================================================
# Use "perl Makefile.PL <options>"
#======================================================================
our @EXCLUSIONS=();
our $MORE_MACROS = {};
our $MORE_MAKERULES=''; 

install_spawner_service();
WriteMakefile(
	NAME => 'CorTeX-Peripherals',
	AUTHOR => 'Deyan Ginev <d.ginev@jacobs-university.de>',
	ABSTRACT => "Peripherals management and process spawning for CorTeX",
	VERSION_FROM => 'lib/CorTeX/Service.pm',
	PREREQ_PM => { 
		"XML::LibXML"=>0,
		"LLaMaPUn"=>0,
		"Net::Address::IP::Local"=>0,
		"AnyEvent::Gearman"=>0,
		"Unix::Processors"=>0,
		"LaTeXML"=>'0.7.99'
	},
	EXE_FILES => ['bin/cortex-spawner','bin/cortex-worker'],
	macro => $MORE_MACROS,
  (eval { ExtUtils::MakeMaker->VERSION(6.46) } ? (META_MERGE => {
    'meta-spec' => { version => 2 },
      resources => {
        repository => {
          type => 'git',
          url => 'https://github.com/dginev/CorTeX-Peripheral.git',
          web => 'https://github.com/dginev/CorTeX-Peripheral',
        }}})
  : ()),
);

#**********************************************************************
# Overriding ExtUtils::MM methods
#**********************************************************************
# Append any additional Makefile rules added by the following.
# Exclude the sources used to generate others from the build (See below).
sub MY::libscan {
	my($self,$path)=@_;
	if(($path =~ /~$/) || grep($path eq $_, @EXCLUSIONS)){
		return ""; }
	$self->MY::SUPER::libscan($path); }
sub MY::postamble {
	shift->MY::SUPER::postamble(@_) . $MORE_MAKERULES; }

sub install_spawner_service {
	push(@EXCLUSIONS,'blib/script/cortex-peripheral');
	$MORE_MAKERULES .= <<'InstallService';
# Copy the spawner service to /etc/init.d/
pure_install :: $(INST_SCRIPT)/cortex-peripheral

$(INST_SCRIPT)/cortex-peripheral: bin/cortex-peripheral
	$(CP) bin/cortex-peripheral /etc/init.d/
InstallService
}