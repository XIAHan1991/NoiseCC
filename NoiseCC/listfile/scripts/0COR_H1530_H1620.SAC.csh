#!/bin/csh
sac << END
r /PARA/pp466/PKUSummer/NoiseCC/200411/5to50_cor/COR/COR_H1530_H1620.SAC
w a

r /PARA/pp466/PKUSummer/NoiseCC/200412/5to50_cor/COR/COR_H1530_H1620.SAC
w aa
r a
addf aa
w a

r a
ch user0 61
w /PARA/pp466/PKUSummer/NoiseCC/stack/COR_H1530_H1620.SAC
END

