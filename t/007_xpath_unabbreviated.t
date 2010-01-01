
use strict;
use Test::More tests => 11; 
use XML::TinyXML;
use XML::TinyXML::Selector;
BEGIN { use_ok('XML::TinyXML::Selector::XPath::Functions') };

my $txml = XML::TinyXML->new();
$txml->loadFile("./t/t.xml");
my $selector = XML::TinyXML::Selector->new($txml, "XPath");
use Data::Dumper;
my ($root) = $selector->_select_unabbreviated("/");
is ($root->name, "xml");

my @set = $selector->_select_unabbreviated("child::*");
is (scalar(@set), 6);

$selector->resetContext;
my ($node) = $selector->_select_unabbreviated("child::parent");
is ($root->name, "xml");

$selector->resetContext;
@set = $selector->_select_unabbreviated("descendant::*");
is (scalar(@set), 11);

($node) = $selector->_select_unabbreviated("/parent/blah");
is ($node->name, "blah");

@set = map { $_->name } $selector->_select_unabbreviated("/parent/blah/ancestor::*");
is_deeply(\@set, [ 'parent', 'xml' ]);

$selector->resetContext;
@set = $selector->_select_unabbreviated("child::parent[child::child1]");
is (scalar(@set), 1);
is ($set[0]->path, "/xml/parent");

$selector->resetContext;
@set = $selector->_select_unabbreviated("child::parent[child::blah='SECOND']");
is (scalar(@set), 1);
is ($set[0]->getChildNode(1)->name, "blah");
