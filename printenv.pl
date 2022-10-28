#!/usr/bin/perl

# source: https://en.wikipedia.org/w/index.php?title=Common_Gateway_Interface&oldid=1115605695#Example

=head1 DESCRIPTION

printenv — a CGI program that just prints its environment

=cut

print "Content-Type: text/plain\n";
print "\n";

for my $var ( sort keys %ENV ) {
    printf "%s=\"%s\"\n", $var, $ENV{$var};
}
