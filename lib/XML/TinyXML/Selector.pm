# -*- tab-width: 4 -*-
# ex: set tabstop=4:

=head1 NAME

XML::TinyXML::Selector - Tinyxml Selector base class

=head1 SYNOPSIS

=over 4

  use XML::TinyXML;

  # first obtain an xml context:
  $xml = XML::TinyXML->new("rootnode", "somevalue", { attr1 => v1, attr2 => v2 });

=back

=head1 DESCRIPTION

Selector base class

=head1 INSTANCE VARIABLES

=over 4

=back

=head1 METHODS

=over 4

=cut
package XML::TinyXML::Selector;

use strict;
our $VERSION = '0.18';

=item * new ($xml, $type)

Creates a new XML::TinyXML::Selector::$type object.

$xml must be a valid XML::TinyXML instance
$type must be a known selector $type

Returns a valid XML::TinyXML::Node object
Returns undef is $type is not known or if $xml is not a valid instance

=cut
sub new {
    my ($class, $xml, $type, %args) = @_;
    return undef
        unless($xml && ref($xml) && UNIVERSAL::isa($xml, "XML::TinyXML"));
    my $module = __PACKAGE__ . "::" . $type;
    if (eval "require $module; 1") {
        my $self = {};
        bless($self, $module);
        $self->{_xml} = $xml;
        return $self->init(%args);
    }
    return undef;
}

1;

=back

=head1 SEE ALSO

=over 4

XML::TinyXML

=back

=head1 AUTHOR

xant, E<lt>xant@cpan.orgE<gt>

=head1 COPYRIGHT AND LICENSE

Copyright (C) 2008 by xant

This library is free software; you can redistribute it and/or modify
it under the same terms as Perl itself, either Perl version 5.8.8 or,
at your option, any later version of Perl 5 you may have available.


=cut
