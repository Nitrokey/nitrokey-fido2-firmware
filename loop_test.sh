
i=0; while make test_simulation; do i=$((i+1)); echo "================+" $i; done; echo $i; notify-send 'test' 'finished'

