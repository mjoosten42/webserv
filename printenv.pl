#!/usr/bin/env perl

# source: https://en.wikipedia.org/w/index.php?title=Common_Gateway_Interface&oldid=1115605695#Example

=head1 DESCRIPTION

printenv — a CGI program that just prints its environment

=cut

sleep 1;

print "Content-Type: text/plain\n\n";

for my $var ( sort keys %ENV ) {
    printf "%s=\"%s\"\n", $var, $ENV{$var};
}
