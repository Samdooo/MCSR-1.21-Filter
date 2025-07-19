# First, split the big seed file into smaller chunks (e.g., 100,000 lines each)
split -l 100000 input seed_chunk_

# Create directories for each chunk and move chunks into them
ls seed_chunk_* | xargs -I {} bash -c 'mkdir -p dir_{}; mv {} dir_{}/input'

# Copy the seedchecker program into each directory and run it in parallel
find dir_* -type d | xargs -P $(nproc) -I {} bash -c 'cp seedchecker {}; cd {}; ./seedchecker > ../results_{}.txt'

# After completion, combine all results
cat results_*.txt > final_results.txt

# Clean up temporary files (optional)
rm -r dir_* results_*.txt