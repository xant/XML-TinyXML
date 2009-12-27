# ex: set tabstop=4:

=head1 NAME

XML::TinyXML::Selector::XPath - XPath-compliant selector for XML::TinyXML

=head1 SYNOPSIS

=over 4

  use XML::TinyXML;

  # first obtain an xml context:
  $xml = XML::TinyXML->new("rootnode", "somevalue", { attr1 => v1, attr2 => v2 });

  $selector = XML::TinyXML::Selector->new($xml, "XPath");

  #####
  Assuming the following xml data :
  <?xml version="1.0"?>
  <xml>
    <hello>world</hello>
    <foo>
      <![CDATA[ this should unescape <&; etc... :) ]]>
    </foo>
    <parent>
      <child1/>
      <child2/>
      <child3/>
    </parent>
    <parent>
      <blah>SECOND</blah>
    </parent>
  </xml>
  #####

  @res = $selector->select('/xml//parent');
  @res = $selector->select('//child*');
  @res = $selector->select('/xml/parent[2]/blah/..');
  @res = $selector->select('//blah/..');
  @res = $selector->select('//parent[1]/..');
  @res = $selector->select('//parent[1]/.');
  @res = $selector->select('//blah/.');

=back

=head1 DESCRIPTION

XPath-compliant selector for XML::TinyXML

=head1 INSTANCE VARIABLES

=over 4

=back

=head1 METHODS

=over 4

=cut
package XML::TinyXML::Selector::XPath;

use strict;
use base qw(XML::TinyXML::Selector);
our $VERSION = '0.14';

=item * init ()

=cut
sub init {
    my ($self, %args) = @_;
    return $self;
}


sub _parse_predicate {
    my ($self, $predicate) = @_;
    my ($attr, $value);
    my %res;
    if ($predicate =~ /^([0-9]+)$/) {
        $res{idx} = $1;
    } elsif (($attr, $value) = $predicate =~ /^\@(\S+)\s*=\s*(.*)\s*$/) {
        $value = substr($value, 1, length($value)-2)
                if ($value =~ /^([\"'])(?:\\\1|.)*?\1$/);
        $res{attr} = $attr;
        $res{attr_value} = $value;
    } elsif (($attr) = $predicate =~ /^\@(\S+)$/) {
        $res{attr} = $attr; 
    }
    # TODO - support all predicates
    return wantarray?%res:\%res;
}


=item * select ($expr, [ $cnode ])

=cut
sub select {
    my ($self, $expr, $cnode) = @_;
    my @path = split('/', $expr);
    my @set;
    if (!$path[0]) { # we are starting from the root
        shift(@path);
    } elsif(!$cnode) { # relative path ... let's get current node
        $cnode = $self->{_xml}->cNode;
    }
    while (@path) {
        my ($attr, $value);
        my $tag = shift(@path);
        if (!$tag) {
            my @found;
            $tag = $path[0];
            my $predicate_string = $tag =~ s/\[(.*?)\]$//
                                 ? $1
                                 : undef;
            my $predicate = $predicate_string
                          ? $self->_parse_predicate($predicate_string)
                          : undef;
            my $idx = $predicate?$predicate->{idx}:0;
            $tag =~ s/\*/\.\*/g;
            foreach my $child ($cnode?$cnode->children:$self->{_xml}->rootNodes) {
                my @selection;
                if (@path < 2) {
                    push (@found, $child) if ($child->name =~ /^$tag$/ && @path < 2);
                    @selection = $self->select(join('/', '/', @path), $child);
                } else {
                    if ($child->name =~ /^$tag$/) {
                        shift(@path);
                        @selection = $self->select(join('/', @path), $child);
                    } else {
                        @selection = $self->select(join('/', '/', @path), $child);
                    }
                }
                push (@found, @selection) if (@selection);
            }

            # check if we are the first, or if we are still inside the recursion 
            # XXX - there should be a better way
            if (caller() ne __PACKAGE__ && $predicate) {
                if ($idx) {
                    push(@set, $found[$idx-1]) if ($found[$idx-1]);
                } elsif ($predicate->{attr}) {
                    foreach my $n (@found) {
                        push (@set, $n) if (
                            (
                             $predicate 
                             && defined($predicate->{attr_value})
                             && defined($n->attributes->{$predicate->{attr}})
                            ) ? $n->attributes->{$predicate->{attr}} eq $predicate->{attr_value} 
                              : defined($n->attributes->{$predicate->{attr}})
                        );
                    }
                }
            } else {
                push(@set, @found);
            }
            last;
        } else {
            if ($tag eq '.') {
                push (@set, $cnode) if($cnode && !@path);
            } elsif ($tag eq '..') {
                push (@set, $cnode->parent) if($cnode && !@path);
                $cnode = $cnode->parent;
            } else {
                if ($tag =~ s/\*/\.\*/g) {
                    foreach my $child ($cnode ? $cnode->children : $self->{_xml}->rootNodes) {
                        if ($child->name =~ /^$tag$/) {
                            if (@path) {
                                my @selection = $self->select(join('/', @path), $child);
                                push (@set, @selection) if (@selection);
                            } else {
                                push (@set, $child);
                            }
                        }
                    }
                    last; # break
                } else {
                    my ($predicate_string) = $tag =~ /\[(.*?)\]$/;
                    # some predicates are already supported by the underlying C library
                    my $predicate = ($predicate_string && $predicate_string !~ /^[0-9]+$/) 
                                  ? $self->_parse_predicate($predicate_string)
                                  : undef;
                    if ($cnode) {
                        $cnode = $cnode->getChildNodeByName($tag);
                    } else {
                        $cnode = $self->{_xml}->getNode("/$tag");
                    }
                    push (@set, $cnode) if($cnode && !@path);
                }
            }
        }
    }
    return wantarray?@set:\@set;
}


1;

=back

=head1 SEE ALSO

=over 4

XML::TinyXML XML::TinyXML::Node XML::TinyXML::Selector

=back

=head1 AUTHOR

xant, E<lt>xant@cpan.orgE<gt>

=head1 COPYRIGHT AND LICENSE

Copyright (C) 2009 by xant

This library is free software; you can redistribute it and/or modify
it under the same terms as Perl itself, either Perl version 5.8.8 or,
at your option, any later version of Perl 5 you may have available.


=cut
