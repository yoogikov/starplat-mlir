#!/bin/bash

# Step 1
echo "Step 1: Running app on buildup.avl..."
if output1=$(./build/bin/app ./testing/buildup.avl > ./testing/bd.mlir 2>&1); then
    echo "✓ Step 1 completed"
else
    echo "✗ Error in Step 1:"
    echo "$output1"
    exit 1
fi

# Step 2
echo "Step 2: Running starplat-opt..."
if output2=$(./build/bin/starplat-opt --starplat-to-llvm-seq ./testing/bd.mlir -o ./testing/lowered_bd.mlir 2>&1); then
    echo "✓ Step 2 completed"
else
    echo "✗ Error in Step 2"
    echo "Previous step output:"
    echo "$output1"
    echo "Current step output:"
    echo "$output2"
    exit 1
fi

# Step 3
echo "Step 3: Running mlir-translate..."
if output3=$(mlir-translate --mlir-to-llvmir ./testing/lowered_bd.mlir -o ./testing/llvm_bd.ll 2>&1); then
    echo "✓ Step 3 completed"
else
    echo "✗ Error in Step 3"
    echo "Step 1 output:"
    echo "$output1"
    echo "Step 2 output:"
    echo "$output2"
    echo "Current step output:"
    echo "$output3"
    exit 1
fi

# Step 4
echo "Step 4: Compiling with clang++..."
if output4=$(clang++ ./testing/llvm_bd.ll ./testing/main.cpp -o ./testing/a.out 2>&1); then
    echo "✓ Step 4 completed"
else
    echo "✗ Error in Step 4"
    echo "Step 1 output:"
    echo "$output1"
    echo "Step 2 output:"
    echo "$output2"
    echo "Step 3 output:"
    echo "$output3"
    echo "Current step output:"
    echo "$output4"
    exit 1
fi

# Step 5
echo "Step 5: Running compiled executable..."
if output5=$(cd ./testing && ./a.out 2>&1); then
    echo "✓ Step 5 completed"
    echo ""
    echo "Final output:"
    echo "$output5"
else
    echo "✗ Error in Step 5"
    echo "Step 1 output:"
    echo "$output1"
    echo "Step 2 output:"
    echo "$output2"
    echo "Step 3 output:"
    echo "$output3"
    echo "Step 4 output:"
    echo "$output4"
    echo "Current step output:"
    echo "$output5"
    exit 1
fi
