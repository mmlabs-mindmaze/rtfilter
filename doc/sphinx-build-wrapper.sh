#!/bin/sh

sphinxbuild=$1
source_root=$2
builder=$3
doc_srcdir=$4
out_dir=$5

export srctree=$source_root
$sphinxbuild -M $builder -d doc/.doctrees -j auto $doc_srcdir $out_dir
