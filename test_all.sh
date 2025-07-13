
set -xe

cd ./Arena
make && ./main >> /dev/null
make clean >> /dev/null
echo ""


# echo "Testing Context"
cd ../context
make && ./main >> /dev/null
make clean >> /dev/null
echo ""


cd ../JSON_Parser
make main && ./main >> /dev/null
make clean >> /dev/null
echo ""


cd ../misc
make tests
./build/file_test >> /dev/null
./build/hashmap_test >> /dev/null
make clean >> /dev/null
echo ""


cd ../Multi_Buddy
make all
./main >> /dev/null && ./main_unity >> /dev/null
make clean >> /dev/null
echo ""


cd ../profiler
make && ./main 10000000 >> /dev/null
make clean >> /dev/null
echo ""


cd ../String_Helper
make main && ./main >> /dev/null
make clean >> /dev/null
echo ""


echo "All Tests Run."
