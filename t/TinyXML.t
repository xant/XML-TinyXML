# Before `make install' is performed this script should be runnable with
# `make test'. After `make install' it should work as `perl TinyXML.t'

#########################

# change 'tests => 2' to 'tests => last_test_to_print';

use Test::More tests => 9;
BEGIN { use_ok('XML::TinyXML') };


my $fail = 0;
foreach my $constname (qw(
	XML_BADARGS XML_GENERIC_ERR XML_LINKLIST_ERR XML_MEMORY_ERR XML_NOERR
	XML_OPEN_FILE_ERR XML_PARSER_GENERIC_ERR XML_UPDATE_ERR XML_NODETYPE_COMMENT
        XML_NODETYPE_SIMPLE XML_NODETYPE_CDATA)) {
  next if (eval "my \$a = $constname; 1");
  if ($@ =~ /^Your vendor has not defined XML::TinyXML macro $constname/) {
    print "# pass: $@";
  } else {
    print "# fail: $@";
    $fail = 1;
  }

}

ok( $fail == 0 , 'Constants' );
#########################

# Insert your test code below, the Test::More module is use()ed here so read
# its man page ( perldoc Test::More ) for help writing this test script.
my $testhash = { 
    a => 'b' , 
    c => 'd', 
    hash => { 
        key1 => 'value1',
        key2 => 'value2' 
    }, 
    array => [ 
        "arrayval1", 
        { subhashkey => 'subhashvalue' }, 
        [  # XXX - sub arrays will be flattened by actual implementation
            { nome1 => 'subarray1' } , 
            { nome2 => 'subarray2' , 'nome2.5' => 'dfsdf'}, 
            { nested => { nested2_1 => 'nestedvalue', nested2_2 => 'nestedvalue2' } },
            "subarrayval1", 
            "subarrayval2" 
        ]
    ]
};

my $txml = XML::TinyXML->new($testhash);

ok( $txml , "XML::TinyXML Object from an hash");

my $newhash = $txml->toHash;
ok( $newhash->{a} eq $testhash->{a}, "simple hash member1" );
ok( $newhash->{c} eq $testhash->{c}, "simple hash member2" );
ok( scalar(keys(%{$newhash->{hash}})) == 2, "nested hash number of members" );
ok( $newhash->{hash}->{key1} eq $testhash->{hash}->{key1}, "nested hash member1" );
ok( $newhash->{hash}->{key2} eq $testhash->{hash}->{key2}, "nested hash member2" );

my $txml3 = XML::TinyXML->new();
$txml3->loadFile("./t/t.xml");
my $out = $txml3->dump;

# here we KNOW that t.xml should have XML::TinyXML to produce exactly the same output
# note that this isn't always true ... since XML::TinyXML never expands leading tabs 
# and, in general, ignores whitespaces (apart those in the value)
open(IN, "./t/t.xml"); 
my $in = "";
while(<IN>) {
    $in .= $_;
}
ok( $out eq $in, "import/export" );
