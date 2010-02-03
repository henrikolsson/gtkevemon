SKILLTREE_ORIG = SkillTree.xml.orig
SKILLTREE_NEW = SkillTree.xml
SKILLTREE_GZIP = ${SKILLTREE_NEW}.gz

CERTTREE_ORIG = CertificateTree.xml.orig
CERTTREE_NEW = CertificateTree.xml
CERTTREE_GZIP = ${CERTTREE_NEW}.gz

SKILLTREE_VERSION = 6
CERTTREE_VERSION = 5

all:
	${MAKE} skilltree
	${MAKE} certtree

skilltree:
	wget -O ${SKILLTREE_ORIG} http://api.eve-online.com/eve/SkillTree.xml.aspx
	cat ${SKILLTREE_ORIG} | perl -w convert_xml.pl ${SKILLTREE_VERSION} > ${SKILLTREE_NEW}
	gzip -c ${SKILLTREE_NEW} > ${SKILLTREE_GZIP}
	rm -f ${SKILLTREE_ORIG} ${SKILLTREE_NEW}

certtree:
	wget -O ${CERTTREE_ORIG} http://api.eve-online.com/eve/CertificateTree.xml.aspx
	cat ${CERTTREE_ORIG} | perl -w convert_xml.pl ${CERTTREE_VERSION} > ${CERTTREE_NEW}
	gzip -c ${CERTTREE_NEW} > ${CERTTREE_GZIP}
	rm -f ${CERTTREE_ORIG} ${CERTTREE_NEW}

clean:
	rm -f ${SKILLTREE_ORIG} ${SKILLTREE_NEW} ${SKILLTREE_GZIP} ${CERTTREE_ORIG} ${CERTTREE_NEW} ${CERTTREE_GZIP}
