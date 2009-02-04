use ExtUtils::MakeMaker;
use 5.008;
# See lib/ExtUtils/MakeMaker.pm for details of how to influence
# the contents of the Makefile that is written.
WriteMakefile(
    NAME              => 'XML::TinyXML',
    VERSION_FROM      => 'lib/XML/TinyXML.pm', # finds $VERSION
    PREREQ_PM         => { }, # e.g., Module::Name => 1.1
    ABSTRACT_FROM  => 'lib/XML/TinyXML.pm', # retrieve abstract from module
    AUTHOR         => 'xant <xant@cpan.org>',
    LIBS              => [''], # e.g., '-lm'
    DEFINE            => '', # e.g., '-DHAVE_SOMETHING'
    INC               => '-I.', # e.g., '-I. -I/usr/include/other'
    OBJECT            => '$(O_FILES)', # link all the C files too
);
if  (eval {require ExtUtils::Constant; 1}) {
  # If you edit these definitions to change the constants used by this module,
  # you will need to use the generated const-c.inc and const-xs.inc
  # files to replace their "fallback" counterparts before distributing your
  # changes.
  my @names = (qw(XML_BADARGS XML_GENERIC_ERR XML_LINKLIST_ERR XML_MEMORY_ERR
		 XML_NOERR XML_OPEN_FILE_ERR XML_PARSER_GENERIC_ERR
		 XML_UPDATE_ERR XML_NODETYPE_SIMPLE XML_NODETYPE_COMMENT XML_NODETYPE_CDATA));
  ExtUtils::Constant::WriteConstants(
                                     NAME         => 'TinyXML',
                                     NAMES        => \@names,
                                     DEFAULT_TYPE => 'IV',
                                     C_FILE       => 'const-c.inc',
                                     XS_FILE      => 'const-xs.inc',
                                  );

}
else {
  use File::Copy;
  use File::Spec;
  foreach my $file ('const-c.inc', 'const-xs.inc') {
    my $fallback = File::Spec->catfile('fallback', $file);
    copy ($fallback, $file) or die "Can't copy $fallback to $file: $!";
  }
}
