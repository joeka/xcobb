Extract or create OBB files for the Android version of XCOM.

Build:
  g++ -I /usr/include/boost -lboost_system -lboost_filesystem xcobb.cpp -o xcobb

Usage:
  Extract:  xcobb x obb_file [output_dir]
  Create:   xcobb c obb_file input_dir
  List:     xcobb l[s] obb_file
            (ls will also print the size)
