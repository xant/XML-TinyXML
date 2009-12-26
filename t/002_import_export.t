
use Test::More tests => 2;
BEGIN { use_ok('XML::TinyXML') };

$txml = XML::TinyXML->new();
$txml->loadFile("./t/t.xml");
my $out = $txml->dump;

# here we KNOW that t.xml should have XML::TinyXML to produce exactly the same output
# note that this isn't always true ... since XML::TinyXML never expands leading tabs 
# and, in general, ignores whitespaces (apart those in the value)
open(IN, "./t/t.xml"); 
my $in = "";
while(<IN>) {
    $in .= $_;
}
ok( $out eq $in, "import/export" );
