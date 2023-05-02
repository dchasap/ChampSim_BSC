

HOME="/gpfs/home/bsc18/bsc18186/scratch/ChampSim-MultiPageSize"

file=$1

echo "total_small_pages,total_large_pages,small_page_percent,large_page_percent"

total_small_pages=$(grep "Total small pages:" ${file} | cut -d ' ' -f 4)
total_large_pages=$(grep "Total large pages:" ${file} | cut -d ' ' -f 4)
small_page_percent=$(( (100 * $total_small_pages) / ($total_small_pages + $total_large_pages) ))
large_page_percent=$(( (100 * $total_large_pages) / ($total_small_pages + $total_large_pages) ))

echo "$total_small_pages,$total_large_pages,$small_page_percent,$large_page_percent"
