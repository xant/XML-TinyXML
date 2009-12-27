
use strict;
use Test::More tests => 18;
use XML::TinyXML;
BEGIN { use_ok('XML::TinyXML::Selector') };

my $txml = XML::TinyXML->new();
$txml->loadFile("./t/t.xml");

my $rnode = $txml->getNode("/xml");
is ($rnode->name, "xml");
my $test = $rnode->getChildNodeByName("parent[2]"); # this tests predicates support within C library
is ($test->name, "parent");
$test = $txml->getNode("/xml/parent[2]/blah"); # this tests predicates support within C library
is ($test->value, "SECOND");

my $selector = XML::TinyXML::Selector->new($txml, "XPath");
my @res = $selector->select('//parent');
is (scalar(@res), 2);
@res = $selector->select('/xml//parent');
is (scalar(@res), 2);
@res = $selector->select('//parent[1]');
is (scalar(@res), 1);
@res = $selector->select('//parent[2]');
is (scalar(@res), 1);
@res = $selector->select('//child*');
is (scalar(@res), 3);

@res = $selector->select('/xml/parent[2]/blah/..');
is ($res[0]->name, "parent");
@res = $selector->select('//blah/..');
is ($res[0]->name, "parent");
@res = $selector->select('//parent[1]/..');
is ($res[0]->name, "xml");
@res = $selector->select('//parent[1]/.');
is ($res[0]->name, "parent");
@res = $selector->select('//blah/.');
is ($res[0]->name, "blah");

my ($node) = $selector->select('//parent[2]');
ok($node->attributes->{attr});
($node) = $selector->select('//parent[@attr]');
ok($node->attributes->{attr});
($node) = $selector->select('//parent[@attr=val]');
ok($node->attributes->{attr});
($node) = $selector->select('/xml/parent[@attr=val]'); # this tests predicates support within C library
ok($node->attributes->{attr});
