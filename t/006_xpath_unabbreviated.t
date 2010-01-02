
use strict;
use Test::More tests => 39;
use XML::TinyXML;
use XML::TinyXML::Selector;
use Data::Dumper;

BEGIN { use_ok('XML::TinyXML::Selector::XPath::Functions') };

my $txml = XML::TinyXML->new();
$txml->loadFile("./t/t.xml");
my $selector = XML::TinyXML::Selector->new($txml, "XPath");

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
is_deeply (\@set, [ 'parent', 'xml' ]);

$selector->resetContext;
@set = $selector->_select_unabbreviated("child::parent[child::child1]");
is (scalar(@set), 1);
is ($set[0]->path, "/xml/parent");

$selector->resetContext;
@set = $selector->_select_unabbreviated("child::parent[child::child1]/preceding-sibling::*");
is (scalar(@set), 3);
is ($set[0]->name, "foo");

$selector->resetContext;
@set = $selector->_select_unabbreviated("child::parent[child::blah='SECOND']");
is (scalar(@set), 1);
is ($set[0]->getChildNode(1)->name, "blah");
@set = $selector->_select_unabbreviated("attribute::*"); # reusing last context node
is (scalar(@set), 1);
is_deeply ([$set[0]->name, $set[0]->value], ['attr', 'val']); # ensure it's the expected attribute
is ($set[0]->{attr}, 'val'); # retro-compatibility check

$selector->resetContext;
@set = $selector->_select_unabbreviated("child::parent[child::blah='SECOND']/following-sibling::*");
is (scalar(@set), 1);
is ($set[0]->name, "qtest");

$selector->resetContext;
@set = $selector->_select_unabbreviated("child::parent[child::blah='NOT EXISTING' or child::child1]/following-sibling::*");
is (scalar(@set), 2);
is_deeply ([ map { $_->name } @set ], [ 'parent', 'qtest' ]);

@set = $selector->_select_unabbreviated("child::parent[child::blah='NOT EXISTING' and child::child1]");
is (scalar(@set), 0);

$selector->resetContext;
@set = $selector->_select_unabbreviated("child::parent[child::blah='SECOND']/attribute::*");
is (scalar(@set), 1);
is ($set[0]->{attr}, 'val');

$selector->resetContext;
@set = $selector->_select_unabbreviated("child::parent[child::blah='SECOND']/attribute::attr");
is (scalar(@set), 1);
is_deeply ([ $set[0]->name, $set[0]->value ], ['attr', 'val']); # ensure it's the expected attribute (again)

$selector->resetContext;
@set = $selector->_select_unabbreviated("child::parent[position()=2]");
is (scalar(@set), 1);
is ($set[0]->getChildNode(1)->name, "blah");

$selector->resetContext;
@set = $selector->_select_unabbreviated("child::parent[position()=last()]");
is (scalar(@set), 1);
is ($set[0]->getChildNode(1)->name, "blah");

$selector->resetContext;
@set = $selector->_select_unabbreviated("child::parent[position()=last()-1]");
is (scalar(@set), 1);
is ($set[0]->getChildNode(1)->name, "child1");

$selector->resetContext;
@set = $selector->_select_unabbreviated("child::parent[position()=1+1]");
is (scalar(@set), 1);
is ($set[0]->getChildNode(1)->name, "blah");
$selector->resetContext;
@set = $selector->_select_unabbreviated("child::parent[position()>=1][position()=2]");
is (scalar(@set), 1);
is ($set[0]->getChildNode(1)->name, "blah");

$selector->resetContext;
@set = $selector->_select_unabbreviated("descendant::*[attribute::attr]");
is (scalar(@set), 2);
is_deeply ([ map { $_->name } @set ], [qw(parent blah)]);

$selector->resetContext;
@set = $selector->_select_unabbreviated("descendant::*[attribute::attr='val2']");
is (scalar(@set), 1);
is_deeply ([ $set[0]->name, $set[0]->value  ], [qw(blah SECOND)]);
