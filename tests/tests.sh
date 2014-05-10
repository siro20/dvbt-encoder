#!bin/bash
make all
LD_LIBRARY_PATH=../libdvbtenc/lib
export LD_LIBRARY_PATH
cat ./ref_2K_4QAM_1_2_32/test.m2v | ./ed -o 2048 -b 8 -c 2 -g 32 -m 4 -a 1 -s 0 > ./output_ed_cmp.bin
if cmp -n $(stat -c %s "./ref_2K_4QAM_1_2_32/output_ed.bin") ./ref_2K_4QAM_1_2_32/output_ed.bin ./output_ed_cmp.bin ; then
  echo pbrs same
else
  echo pbrs different
fi
rm ./output_ed_cmp.bin

cat ./ref_2K_4QAM_1_2_32/output_ed.bin | ./rs -o 2048 -b 8 -c 2 -g 32 -m 4 -a 1 -s 0 > ./output_rs_cmp.bin
if cmp -n $(stat -c %s "./ref_2K_4QAM_1_2_32/output_rs.bin") -b ./ref_2K_4QAM_1_2_32/output_rs.bin ./output_rs_cmp.bin; then
  echo rs same
else
  echo rs different
fi
rm ./output_rs_cmp.bin

cat ./ref_2K_4QAM_1_2_32/output_rs.bin | ./oi -o 2048 -b 8 -c 2 -g 32 -m 4 -a 1 -s 0 > ./output_ci_cmp.bin
if cmp -n $(stat -c %s "./ref_2K_4QAM_1_2_32/output_ci.bin") -b ./ref_2K_4QAM_1_2_32/output_ci.bin ./output_ci_cmp.bin ; then
  echo ci same
else
  echo ci different
fi
rm ./output_ci_cmp.bin

cat ./ref_2K_4QAM_1_2_32/output_ci.bin | ./ce -o 2048 -b 8 -c 2 -g 32 -m 2 -a 1 -s 0 > ./output_ve_cmp.bin
if cmp -n $(stat -c %s "./ref_2K_4QAM_1_2_32/output_ve.bin") -b ./ref_2K_4QAM_1_2_32/output_pu.bin ./output_ve_cmp.bin ; then
  echo ce same
else
  echo ce different
fi
rm ./output_ve_cmp.bin

cat ./ref_2K_4QAM_1_2_32/output_pu.bin | ./ii -o 2048 -b 8 -c 2 -g 32 -m 2 -a 1 -s 0 > ./output_bi_cmp.bin
if cmp -b -n $(stat -c %s "./ref_2K_4QAM_1_2_32/output_bi.bin") -b ./ref_2K_4QAM_1_2_32/output_bi.bin ./output_bi_cmp.bin ; then
  echo bi same
else
  echo bi different
fi
rm ./output_bi_cmp.bin

cat ./ref_2K_4QAM_1_2_32/output_bi.bin | ./si -o 2048 -b 8 -c 2 -g 32 -m 2 -a 1 -s 0 > ./output_si_cmp.bin
if cmp -b -n $(stat -c %s "./ref_2K_4QAM_1_2_32/output_si.bin") -b ./ref_2K_4QAM_1_2_32/output_si.bin ./output_si_cmp.bin ; then
  echo si same
else
  echo si different
fi
rm ./output_si_cmp.bin

cat ./ref_2K_4QAM_1_2_32/output_si.bin | ./sm -o 2048 -b 8 -c 2 -g 32 -m 2 -a 1 -s 0 > ./output_ma_cmp.bin
if cmp -b -n $(stat -c %s "./ref_2K_4QAM_1_2_32/output_ma.bin") -b ./ref_2K_4QAM_1_2_32/output_ma.bin ./output_ma_cmp.bin ; then
  echo ma same
else
  echo ma different
fi
rm ./output_ma_cmp.bin

cat ./ref_2K_4QAM_1_2_32/output_ma.bin | ./chan -o 2048 -b 8 -c 2 -g 32 -m 2 -a 1 -s 0 > ./output_fd_cmp.bin
if cmp -b -n $(stat -c %s "./ref_2K_4QAM_1_2_32/output_fd.bin") -b ./ref_2K_4QAM_1_2_32/output_fd.bin ./output_fd_cmp.bin ; then
  echo chan same
else
  echo chan different
fi
rm ./output_fd_cmp.bin

cat ./ref_2K_4QAM_1_2_32/output_fd.bin | ./ifft -o 2048 -b 8 -c 2 -g 32 -m 2 -a 1 -s 0 > ./output_cmp.bin
if ./float_cmp ./ref_2K_4QAM_1_2_32/output.bin ./output_cmp.bin 0.25 ; then
  echo ifft same
else
  echo ifft different
fi
rm ./output_cmp.bin

cat ./ref_2K_4QAM_1_2_32/test.m2v |./dvbtenc -o 2048 -b 8 -c 2 -g 32 -m 2 -a 1 -s 0 > ./output.bin
if cmp -b -n $(stat -c %s "./ref_2K_4QAM_1_2_32/output.bin") -b ./ref_2K_4QAM_1_2_32/output.bin ./output.bin ; then
  echo dvbtenc same
else
  echo dvbtenc different
fi
#rm ./output.bin
