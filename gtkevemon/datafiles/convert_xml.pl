#! /usr/bin/perl -w

#
# Script to create a new SkillTree.xml and CertificateTree.xml
# file with version information. It simpily appends a dataVersion
# attribute to the <eveapi> tag.
#
# Written by Simon Fuhrmann
#

use strict;

if (@ARGV != 1)
{
  print "Usage: $0 <Version>\n";
  exit;
}

my $version = $ARGV[0];
sub mychomp { $_[0] =~ s/\r?\n$// }

my @contents = <STDIN>;

# Remove tabs, insert version information
foreach my $line (@contents)
{
  mychomp($line);
  my $xml_line = $line;
  $xml_line =~ s/<eveapi (.*)>/<eveapi $1 dataVersion=\"$version\">/;
  $xml_line =~ s/\t//g;
  print "$xml_line\n";
}
