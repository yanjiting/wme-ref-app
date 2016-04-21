for file in *.raw; do
    mv "$file" "`basename $file .raw`.pcm"
done
