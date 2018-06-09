# Blagominer
I've modded Blagominer by modding a modded version from Quibus :-)
Added a couple of thing to Blagominer:
-automatically switch from POC1 to POC2 depending on block height
-added config item "POC2StartBlock":502000 
-added second cache parameter as shuffling needs more cache to perform (less seeks)
-added config item "CacheSize2" : 262144
-added ability to include POC2 filenames (no stagger in filename)
-ability to run the miner on mixed POC1 & POC2 files
-abbility to run POC2 files in a POC1 world & vice versa
-reading in hashing now performed in parallel
