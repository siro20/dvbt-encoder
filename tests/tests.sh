#!bin/bash
make all
LD_LIBRARY_PATH=../libdvbtenc/lib
export LD_LIBRARY_PATH
arguments="-o 2048 -b 8 -c 2 -g 32 -m 2 -a 1 -s 4"
folderpath="ref_2K_4QAM_1_2_32"
echo "****************"
echo "testing 2K mode, bandwidth 8Mhz, coderate 1/2, guardinterval 1/32, modulation qpsk, alpha 1, outputformat float"
cat ./$folderpath/test.m2v | ./ed $arguments > /tmp/output_ed_cmp.bin
if cmp -n $(stat -c %s "./$folderpath/output_ed.bin") ./$folderpath/output_ed.bin /tmp/output_ed_cmp.bin ; then
  echo ed same
else
  echo ed different
fi
rm /tmp/output_ed_cmp.bin

cat ./$folderpath/output_ed.bin | ./rs $arguments > /tmp/output_rs_cmp.bin
if cmp -n $(stat -c %s "./$folderpath/output_rs.bin") -b ./$folderpath/output_rs.bin /tmp/output_rs_cmp.bin; then
  echo rs same
else
  echo rs different
fi
rm /tmp/output_rs_cmp.bin

cat ./$folderpath/output_rs.bin | ./oi $arguments > /tmp/output_ci_cmp.bin
if cmp -n $(stat -c %s "./$folderpath/output_ci.bin") -b ./$folderpath/output_ci.bin /tmp/output_ci_cmp.bin ; then
  echo ci same
else
  echo ci different
fi
rm /tmp/output_ci_cmp.bin

cat ./$folderpath/test.m2v | ./ed_rs_oi $arguments > /tmp/output_ed_rs_oi_cmp.bin
if cmp -n $(stat -c %s "./$folderpath/output_ci.bin") -b ./$folderpath/output_ci.bin /tmp/output_ed_rs_oi_cmp.bin ; then
  echo ed_rs_oi same
else
  echo ed_rs_oi different
fi
rm /tmp/output_ed_rs_oi_cmp.bin

cat ./$folderpath/output_ci.bin | ./ce $arguments > /tmp/output_pu_cmp.bin
if cmp -n $(stat -c %s "./$folderpath/output_pu.bin") -b ./$folderpath/output_pu.bin /tmp/output_pu_cmp.bin ; then
  echo ce+puncturing same
else
  echo ce+puncturing different
fi
rm /tmp/output_pu_cmp.bin

cat ./$folderpath/output_pu.bin | ./ii $arguments > /tmp/output_bi_cmp.bin
if cmp -b -n $(stat -c %s "./$folderpath/output_bi.bin") -b ./$folderpath/output_bi.bin /tmp/output_bi_cmp.bin ; then
  echo bi same
else
  echo bi different
fi
rm /tmp/output_bi_cmp.bin

cat ./$folderpath/output_bi.bin | ./si $arguments > /tmp/output_si_cmp.bin
if cmp -b -n $(stat -c %s "./$folderpath/output_si.bin") -b ./$folderpath/output_si.bin /tmp/output_si_cmp.bin ; then
  echo si same
else
  echo si different
fi
rm /tmp/output_si_cmp.bin

cat ./$folderpath/output_si.bin | ./sm $arguments > /tmp/output_ma_cmp.bin
if cmp -b -n $(stat -c %s "./$folderpath/output_ma.bin") -b ./$folderpath/output_ma.bin /tmp/output_ma_cmp.bin ; then
  echo ma same
else
  echo ma different
fi
rm /tmp/output_ma_cmp.bin

cat ./$folderpath/output_bi.bin | ./si_sm $arguments > /tmp/output_si_sm_cmp.bin
if cmp -b -n $(stat -c %s "./$folderpath/output_ma.bin") -b ./$folderpath/output_ma.bin /tmp/output_si_sm_cmp.bin ; then
  echo si_sm same
else
  echo si_sm different
fi
rm /tmp/output_si_sm_cmp.bin

cat ./$folderpath/output_ma.bin | ./chan $arguments > /tmp/output_fd_cmp.bin
if cmp -b -n $(stat -c %s "./$folderpath/output_fd.bin") -b ./$folderpath/output_fd.bin /tmp/output_fd_cmp.bin ; then
  echo chan same
else
  echo chan different
fi
rm /tmp/output_fd_cmp.bin

cat ./$folderpath/output_fd.bin | ./ifft $arguments > /tmp/output_cmp.bin
if ./float_cmp ./$folderpath/output.bin /tmp/output_cmp.bin 0.25 ; then
  echo ifft same
else
  echo ifft different
fi
rm /tmp/output_cmp.bin

cat ./$folderpath/output_ma.bin | ./chan_ifft $arguments > /tmp/output_cmp.bin
if ./float_cmp ./$folderpath/output.bin /tmp/output_cmp.bin 0.25 ; then
  echo chan_ifft same
else
  echo chan_ifft different
fi
rm /tmp/output_cmp.bin

cat ./$folderpath/test.m2v | ./dvbtenc $arguments > /tmp/output.bin
if ./float_cmp ./$folderpath/output.bin /tmp/output.bin 0.25 ; then
  echo dvbtenc same
else
  echo dvbtenc different
fi
rm /tmp/output.bin

cat ./$folderpath/output_ma.bin | ./chan_ifft_quant $arguments > /tmp/output.bin
if ./float_cmp ./$folderpath/output.bin /tmp/output.bin 0.25 ; then
  echo chan_ifft_quant same
else
  echo chan_ifft_quant different
fi
rm /tmp/output.bin

arguments="-o 8192 -b 7 -c 3 -g 8 -m 4 -a 1 -s 4"
folderpath="ref_8K_16QAM_2_3_8"
echo "****************"
echo "testing 8K mode, bandwidth 7Mhz, coderate 2/3, guardinterval 1/8, modulation 16QAM, alpha 1, outputformat float"
cat ./$folderpath/test.m2v | ./ed $arguments > /tmp/output_ed_cmp.bin
if cmp -n $(stat -c %s "./$folderpath/output_ed.bin") ./$folderpath/output_ed.bin /tmp/output_ed_cmp.bin ; then
  echo ed same
else
  echo ed different
fi
rm /tmp/output_ed_cmp.bin

cat ./$folderpath/output_ed.bin | ./rs $arguments > /tmp/output_rs_cmp.bin
if cmp -n $(stat -c %s "./$folderpath/output_rs.bin") -b ./$folderpath/output_rs.bin /tmp/output_rs_cmp.bin; then
  echo rs same
else
  echo rs different
fi
rm /tmp/output_rs_cmp.bin

cat ./$folderpath/output_rs.bin | ./oi $arguments > /tmp/output_ci_cmp.bin
if cmp -n $(stat -c %s "./$folderpath/output_ci.bin") -b ./$folderpath/output_ci.bin /tmp/output_ci_cmp.bin ; then
  echo ci same
else
  echo ci different
fi
rm /tmp/output_ci_cmp.bin

cat ./$folderpath/test.m2v | ./ed_rs_oi $arguments > /tmp/output_ed_rs_oi_cmp.bin
if cmp -n $(stat -c %s "./$folderpath/output_ci.bin") -b ./$folderpath/output_ci.bin /tmp/output_ed_rs_oi_cmp.bin ; then
  echo ed_rs_oi same
else
  echo ed_rs_oi different
fi
rm /tmp/output_ed_rs_oi_cmp.bin

cat ./$folderpath/output_ci.bin | ./ce $arguments > /tmp/output_pu_cmp.bin
if cmp -n $(stat -c %s "./$folderpath/output_pu.bin") -b ./$folderpath/output_pu.bin /tmp/output_pu_cmp.bin ; then
  echo ce+puncturing same
else
  echo ce+puncturing different
fi
rm /tmp/output_pu_cmp.bin

cat ./$folderpath/output_pu.bin | ./ii $arguments > /tmp/output_bi_cmp.bin
if cmp -b -n $(stat -c %s "./$folderpath/output_bi.bin") -b ./$folderpath/output_bi.bin /tmp/output_bi_cmp.bin ; then
  echo bi same
else
  echo bi different
fi
rm /tmp/output_bi_cmp.bin

cat ./$folderpath/output_bi.bin | ./si $arguments > /tmp/output_si_cmp.bin
if cmp -b -n $(stat -c %s "./$folderpath/output_si.bin") -b ./$folderpath/output_si.bin /tmp/output_si_cmp.bin ; then
  echo si same
else
  echo si different
fi
rm /tmp/output_si_cmp.bin

cat ./$folderpath/output_si.bin | ./sm $arguments > /tmp/output_ma_cmp.bin
if cmp -b -n $(stat -c %s "./$folderpath/output_ma.bin") -b ./$folderpath/output_ma.bin /tmp/output_ma_cmp.bin ; then
  echo ma same
else
  echo ma different
fi
rm /tmp/output_ma_cmp.bin

cat ./$folderpath/output_bi.bin | ./si_sm $arguments > /tmp/output_si_sm_cmp.bin
if cmp -b -n $(stat -c %s "./$folderpath/output_ma.bin") -b ./$folderpath/output_ma.bin /tmp/output_si_sm_cmp.bin ; then
  echo si_sm same
else
  echo si_sm different
fi
rm /tmp/output_si_sm_cmp.bin

cat ./$folderpath/output_ma.bin | ./chan $arguments > /tmp/output_fd_cmp.bin
if cmp -b -n $(stat -c %s "./$folderpath/output_fd.bin") -b ./$folderpath/output_fd.bin /tmp/output_fd_cmp.bin ; then
  echo chan same
else
  echo chan different
fi
rm /tmp/output_fd_cmp.bin

cat ./$folderpath/output_fd.bin | ./ifft $arguments > /tmp/output_cmp.bin
if ./float_cmp ./$folderpath/output.bin /tmp/output_cmp.bin 10 ; then
  echo ifft same
else
  echo ifft different
fi
rm /tmp/output_cmp.bin

cat ./$folderpath/output_ma.bin | ./chan_ifft $arguments > /tmp/output_cmp.bin
if ./float_cmp ./$folderpath/output.bin /tmp/output_cmp.bin 10 ; then
  echo chan_ifft same
else
  echo chan_ifft different
fi
rm /tmp/output_cmp.bin

cat ./$folderpath/test.m2v | ./dvbtenc $arguments > /tmp/output.bin
if ./float_cmp ./$folderpath/output.bin /tmp/output.bin 10 ; then
  echo dvbtenc same
else
  echo dvbtenc different
fi
rm /tmp/output.bin

cat ./$folderpath/output_ma.bin | ./chan_ifft_quant $arguments > /tmp/output.bin
if ./float_cmp ./$folderpath/output.bin /tmp/output.bin 10 ; then
  echo chan_ifft_quant same
else
  echo chan_ifft_quant different
fi
rm /tmp/output.bin

arguments="-o 2048 -b 6 -c 4 -g 4 -m 6 -a 1 -s 4"
folderpath="ref_2K_64QAM_3_4_4"
echo "****************"
echo "testing 2K mode, bandwidth 6Mhz, coderate 3/4, guardinterval 1/4, modulation 64QAM, alpha 1, outputformat float"
cat ./$folderpath/test.m2v | ./ed $arguments > /tmp/output_ed_cmp.bin
if cmp -n $(stat -c %s "./$folderpath/output_ed.bin") ./$folderpath/output_ed.bin /tmp/output_ed_cmp.bin ; then
  echo ed same
else
  echo ed different
fi
rm /tmp/output_ed_cmp.bin

cat ./$folderpath/output_ed.bin | ./rs $arguments > /tmp/output_rs_cmp.bin
if cmp -n $(stat -c %s "./$folderpath/output_rs.bin") -b ./$folderpath/output_rs.bin /tmp/output_rs_cmp.bin; then
  echo rs same
else
  echo rs different
fi
rm /tmp/output_rs_cmp.bin

cat ./$folderpath/output_rs.bin | ./oi $arguments > /tmp/output_ci_cmp.bin
if cmp -n $(stat -c %s "./$folderpath/output_ci.bin") -b ./$folderpath/output_ci.bin /tmp/output_ci_cmp.bin ; then
  echo ci same
else
  echo ci different
fi
rm /tmp/output_ci_cmp.bin

cat ./$folderpath/test.m2v | ./ed_rs_oi $arguments > /tmp/output_ed_rs_oi_cmp.bin
if cmp -n $(stat -c %s "./$folderpath/output_ci.bin") -b ./$folderpath/output_ci.bin /tmp/output_ed_rs_oi_cmp.bin ; then
  echo ed_rs_oi same
else
  echo ed_rs_oi different
fi
rm /tmp/output_ed_rs_oi_cmp.bin

cat ./$folderpath/output_ci.bin | ./ce $arguments > /tmp/output_pu_cmp.bin
if cmp -n $(stat -c %s "./$folderpath/output_pu.bin") -b ./$folderpath/output_pu.bin /tmp/output_pu_cmp.bin ; then
  echo ce+puncturing same
else
  echo ce+puncturing different
fi
rm /tmp/output_pu_cmp.bin

cat ./$folderpath/output_pu.bin | ./ii $arguments > /tmp/output_bi_cmp.bin
if cmp -b -n $(stat -c %s "./$folderpath/output_bi.bin") -b ./$folderpath/output_bi.bin /tmp/output_bi_cmp.bin ; then
  echo bi same
else
  echo bi different
fi
rm /tmp/output_bi_cmp.bin

cat ./$folderpath/output_bi.bin | ./si $arguments > /tmp/output_si_cmp.bin
if cmp -b -n $(stat -c %s "./$folderpath/output_si.bin") -b ./$folderpath/output_si.bin /tmp/output_si_cmp.bin ; then
  echo si same
else
  echo si different
fi
rm /tmp/output_si_cmp.bin

cat ./$folderpath/output_si.bin | ./sm $arguments > /tmp/output_ma_cmp.bin
if cmp -b -n $(stat -c %s "./$folderpath/output_ma.bin") -b ./$folderpath/output_ma.bin /tmp/output_ma_cmp.bin ; then
  echo ma same
else
  echo ma different
fi
rm /tmp/output_ma_cmp.bin

cat ./$folderpath/output_bi.bin | ./si_sm $arguments > /tmp/output_si_sm_cmp.bin
if cmp -b -n $(stat -c %s "./$folderpath/output_ma.bin") -b ./$folderpath/output_ma.bin /tmp/output_si_sm_cmp.bin ; then
  echo si_sm same
else
  echo si_sm different
fi
rm /tmp/output_si_sm_cmp.bin

cat ./$folderpath/output_ma.bin | ./chan $arguments > /tmp/output_fd_cmp.bin
if cmp -b -n $(stat -c %s "./$folderpath/output_fd.bin") -b ./$folderpath/output_fd.bin /tmp/output_fd_cmp.bin ; then
  echo chan same
else
  echo chan different
fi
rm /tmp/output_fd_cmp.bin

cat ./$folderpath/output_fd.bin | ./ifft $arguments > /tmp/output_cmp.bin
if ./float_cmp ./$folderpath/output.bin /tmp/output_cmp.bin 0.25 ; then
  echo ifft same
else
  echo ifft different
fi
rm /tmp/output_cmp.bin

cat ./$folderpath/output_ma.bin | ./chan_ifft $arguments > /tmp/output_cmp.bin
if ./float_cmp ./$folderpath/output.bin /tmp/output_cmp.bin 0.25 ; then
  echo chan_ifft same
else
  echo chan_ifft different
fi
rm /tmp/output_cmp.bin

cat ./$folderpath/test.m2v | ./dvbtenc $arguments > /tmp/output.bin
if ./float_cmp ./$folderpath/output.bin /tmp/output.bin 0.25 ; then
  echo dvbtenc same
else
  echo dvbtenc different
fi
rm /tmp/output.bin

cat ./$folderpath/output_ma.bin | ./chan_ifft_quant $arguments > /tmp/output.bin
if ./float_cmp ./$folderpath/output.bin /tmp/output.bin 0.25 ; then
  echo chan_ifft_quant same
else
  echo chan_ifft_quant different
fi
rm /tmp/output.bin
