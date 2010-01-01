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
use XML::TinyXML::Selector::XPath::Context;
use XML::TinyXML::Selector::XPath::Functions;
use XML::TinyXML::Selector::XPath::Axis;

our $VERSION = '0.14';


sub unimplemented
{
    die "Unimplemented";
}

our @ExprTokens = ('(', ')', '[', ']', '.', '..', '@', ',', '::');

my @NODE_FUNCTIONS = qw(
    last
    position
    count
    id
    local-name
    namespace-uri
    name
);

my @STRING_FUNCTIONS = qw(
    string
    concat
    starts-with
    contains
    substring-before
    substring-after
    substring
    string-length
    normalize-space
    translate
    boolean
    not
    true
    false
    lang
);

my @NUMBER_FUNCTIONS = qw(
    number
    sum
    floor
    ceiling
    round
);

our @AllFunctions = (@NODE_FUNCTIONS, @STRING_FUNCTIONS, @NUMBER_FUNCTIONS);

our @Axis = qw(
    child
    descendant
    parent
    ancestor
    following-sibling
    preceding-sibling
    following
    preceding
    attribute
    namespace
    self
    descendant-or-self
    ancestor-or-self
);

=item * init ()

=cut
sub init {
    my ($self, %args) = @_;
    $self->{context} = XML::TinyXML::Selector::XPath::Context->new($self->{_xml});
    return $self;
}

=item * select ($expr, [ $cnode ])

=cut
sub select {
    my ($self, $expr) = @_;
    my $set;
    if ($expr =~ /::/) { # unabbreviated
        $set = $self->_select_unabbreviated($expr);
    } else {
        $set = $self->_select_abbreviated($expr);
    }
    return wantarray?@$set:$set if ($set);
}

sub context {
    my $self = shift;
    return $self->{context};
}

sub functions {
    my $self = shift;
    return wantarray?@AllFunctions:__PACKAGE__."::Functions";
}

sub resetContext {
    my $self = shift;
    $self->{context} = XML::TinyXML::Selector::XPath::Context->new($self->{_xml});
}

###### PRIVATE METHODS ######
sub _exec_function {
    my ($self, $fun, @args) = @_;
    unless(grep(/^$fun$/, @AllFunctions)) {
        warn "Unsupported Function: '$fun'";
        return undef;
    }
    $fun =~ s/-/_/g;
    return XML::TinyXML::Selector::XPath::Functions->$fun($self->{context}, @args);
}

# Priveate method
sub _expand_axis {
    my ($self, $axis) = @_;
    if ($axis =~ /(\S+)\s+(\S+)\s+(\S+)/) {
        my $a1 = $1;
        my $op = $2;
        my $a2 = $3;

        my $i1 = $self->_expand_axis($a1);
        my $i2 = $self->_expand_axis($a2);
        return $self->context->operators->{$op}->($i1, $i2);
    } else {
        unless(grep(/^$axis$/, @Axis)) {
            warn "Unsupported Axis: '$axis'";
            return undef;
        }
        $axis =~ s/-/_/g;
        return XML::TinyXML::Selector::XPath::Axis->$axis($self->{context});
    }
}

sub _unescape {
    my ($self, $string) = @_;

    $string = substr($string, 1, length($string)-2)
            if ($string =~ /^([\"'])(?:\\\1|.)*?\1$/);
    $string =~ s/&quot;/"/g;
    $string =~ s/&apos;/'/g;
    $string =~ s/&amp;/&/g;
    $string =~ s/&gt;/>/g;
    $string =~ s/&lt;/</g;

    return $string;
}

# Priveate method
sub _parse_predicate {
    my ($self, $predicate) = @_;
    my ($attr, $child, $value);
    my %res;
    if ($predicate =~ /^([0-9]+)$/) {
        $res{idx} = $1;
    } elsif (($attr, $value) = $predicate =~ /^\@(\S+)\s*=\s*(.*)\s*$/) {
        $res{attr} = $attr;
        $res{attr_value} = $self->_unescape($value);
    } elsif (($child, $value) = $predicate =~ /^(\S+)\s*=\s*(.*)\s*$/) {
        $res{child} = $child;
        $res{child_value} = $self->_unescape($value);
    } elsif (($attr) = $predicate =~ /^\@(\S+)$/) {
        $res{attr} = $attr; 
    }
    # TODO - support all predicates
    return wantarray?%res:\%res;
}

sub _select_unabbreviated {
    my ($self, $expr, $recurse) = @_;
    my @tokens = split('/', $expr);
    my @set;
    if ($expr =~ /^\// and !$recurse) { # absolute path has been requested
        $self->{context}->{items} = [$self->{_xml}->rootNodes()];
    }
    #shift(@tokens)
    #    if (!$tokens[0] and $recurse);
    my $token = shift @tokens;
    if ($token and $token =~ /([\w-]+)::([\w\(\)]+|\*)(\[.*?\])?/) {
        my $step = $1;
        my $nodetest = $2;
        my $predicate_string = $3;
        @set = $self->_expand_axis($step);
        if ($nodetest eq '*') {
            $self->context->{items} = \@set;
        } else {
            $self->context->{items} = []; 
            foreach my $node (@set) {
                push (@{$self->context->{items}}, $node) if ($node->name eq $nodetest);
            }
            if ($predicate_string and $predicate_string =~ s/^\[(.*?)\]$/$1/) {
                if ($predicate_string =~ /::/) {
                    my %uniq;
                    foreach my $node ($self->_select_unabbreviated($predicate_string ,1)) {
                        my $parent = $node->parent;
                        if ($parent) {
                            $uniq{$parent->path} = $parent;
                        } else {
                            # TODO - Error Messages
                        }
                    }
                    $self->context->{items} = [ map { $uniq{$_} } keys %uniq ];
                } else {
                    my $predicate = $self->_parse_predicate($predicate_string);
                    if ($predicate->{attr}) {
                    } elsif ($predicate->{child}) { 
                        if ($predicate->{child} =~ s/\(.*?\)//) {
                            @set = $self->_exec_function($predicate->{child});
                        } else {
                        }
                    }
                }
            } else {
                # TODO - Error messages
            }
        }
    } else {
        my @newItems;
        foreach my $node (@{$self->context->items}) {
            if ($token) {
                foreach my $child ($node->children) {
                    push(@newItems, $child)
                        if ($child->name eq $token);
                }
            } else {
                push(@newItems, $node);
            }
        }
        $self->context->{items} = \@newItems;
    }
    if (@tokens) {
        return $self->_select_unabbreviated(join('/', @tokens), 1); # recursion here
    }
    return wantarray?@{$self->context->items}:$self->context->items;
}

sub _select_abbreviated {
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
                    @selection = $self->_select_abbreviated(join('/', '/', @path), $child);
                } else {
                    if ($child->name =~ /^$tag$/) {
                        shift(@path);
                        @selection = $self->_select_abbreviated(join('/', @path), $child);
                    } else {
                        @selection = $self->_select_abbreviated(join('/', '/', @path), $child);
                    }
                }
                push (@found, @selection) if (@selection);
            }

            if (!$cnode && $predicate) {
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
                                my @selection = $self->_select_abbreviated(join('/', @path), $child);
                                push (@set, @selection) if (@selection);
                            } else {
                                push (@set, $child);
                            }
                        }
                    }
                    last; # break
                } else {
                    my ($predicate_string) = $tag =~ /\[(.*?)\]$/;
                    # some predicates are already supported by the underlying C library.
                    # it supports any of : [num], [@attr] and [@attr=value]
                    my $predicate = ($predicate_string && $predicate_string !~ /^(\@\S+|[0-9]+)$/) 
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

