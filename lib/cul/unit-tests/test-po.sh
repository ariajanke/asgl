#correct-po-out.txt
rm -f unit-tests/po-out.txt
./unit-tests/.tpo -bc >> unit-tests/po-out.txt
./unit-tests/.tpo - -cd >> unit-tests/po-out.txt
./unit-tests/.tpo --option-a cat --option-b 1 2 3 4 >> unit-tests/po-out.txt
./unit-tests/.tpo 3 4 --option-b 1 2 >> unit-tests/po-out.txt
./unit-tests/.tpo --option-b - 1 2 3 >> unit-tests/po-out.txt
./unit-tests/.tpo --option-b -- 1 2 3 >> unit-tests/po-out.txt
./unit-tests/.tpo --option-c --option-d >> unit-tests/po-out.txt
diff unit-tests/po-out.txt unit-tests/correct-po-out.txt
