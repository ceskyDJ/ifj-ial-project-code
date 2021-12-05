#!/bin/bash

tmp_file="./build/points-counter/cached-data"
echo "" >$tmp_file

url="https://api.github.com"
token=$GITHUB_TOKEN

org="ceskyDJ"
repos=("ifj-ial-project-code" "ifj-ial-project-docs")
state="all"

authors=(ceskyDJ omnitex TenebrisCZ)
types=("0%20points" "1%20point" "2%20points" "3%20points")
task_points=(0 0 0)
pr_points=(0 0 0)

function get_data() {
  local i=0
  for repo in "${repos[@]}"; do
    curl -sH "Authorization: token $token" -H "Accept: application/vnd.github.v3+json" -H "Content-Type: application/json" "$url/repos/$org/$repo/issues?state=$state&per_page=100$1" >"$tmp_file.$i"
    ((i += 1))
  done

  jq -s 'add' "$tmp_file.0" "$tmp_file.1" >$tmp_file
}

if [ "$token" == "" ]; then
  echo "You need to set your Github token into GITHUB_TOKEN env var" >&2
  exit 1
fi

i=0
for user in "${authors[@]}"; do
  echo "User: $user"

  pr_count=(0 0 0 0 0)
  task_count=(0 0 0 0 0)
  j=0
  for type in "${types[@]}"; do
    get_data "&labels=$type&assignee=$user"
    count=$(jq "length" $tmp_file)

    for ((k = 0; k < count; k++)); do
      # When there are more assignees at the task / pull request,
      # points are distributed to all of them
      # Each person has x/N points, where x = points for the task/PR
      # and N = number of assignees
      assignees_num=$(jq ".[$k] .assignees | length" $tmp_file)
      person_points=$(bc -l <<< "$j/$assignees_num")
      index=$(printf "%.0f" "$person_points")

      if [[ $(jq ".[$k] .pull_request" $tmp_file) != "null" ]]; then
        if [[ "$assignees_num" == 1 ]]; then
          ((pr_count[index] += 1))
        else
          ((pr_count[4] += 1))
        fi
        pr_points[$i]=$(bc -l <<< "${pr_points[$i]}+$person_points")
      else
        if [[ "$assignees_num" == 1 ]]; then
          ((task_count[index] += 1))
        else
          ((task_count[4] += 1))
        fi
        task_points[$i]=$(bc -l <<< "${task_points[$i]}+$person_points")
      fi
    done

    echo "$j point(s):"
    echo " - Task: ${task_count[$j]}x"
    echo " - PR: ${pr_count[$j]}x"

    ((j += 1))
  done

  echo "Partial points:"
  echo " - Task: ${task_count[4]}"
  echo " - PR: ${pr_count[4]}"

  echo "Final points summary:"
  echo " - Task: $(printf "%.2f" "${task_points[$i]}")"
  echo " - PR: $(printf "%.2f" "${pr_points[$i]}")"
  echo ""

  ((i += 1))
done

points_avg=()
i=0
for user in "${authors[@]}"; do
  points_avg[$i]=$(bc -l <<<"(${pr_points[$i]}+${task_points[$i]})/2")
  ((i += 1))
done

avg_points_sum=0
i=0
for user in "${authors[@]}"; do
  avg_points_sum=$(bc -l <<<"$avg_points_sum+${points_avg[$i]}")
  ((i += 1))
done

task_points_sum=0
i=0
for user in "${authors[@]}"; do
  task_points_sum=$(bc -l <<< "$task_points_sum+${task_points[$i]}")
  ((i += 1))
done

pr_points_sum=0
i=0
for user in "${authors[@]}"; do
  pr_points_sum=$(bc -l <<< "$pr_points_sum+${pr_points[$i]}")
  ((i += 1))
done

echo "Points division by tasks:"
i=0
user_perc=0
for user in "${authors[@]}"; do
  user_perc=$(printf "%.0f" "$(bc -l <<<${task_points[$i]}/$task_points_sum*100)")

  echo " - ${user}: ${user_perc}%"
  ((i += 1))
done

echo "Points division by pull requests:"
i=0
user_perc=0
for user in "${authors[@]}"; do
  user_perc=$(printf "%.0f" "$(bc -l <<<${pr_points[$i]}/$pr_points_sum*100)")

  echo " - ${user}: ${user_perc}%"
  ((i += 1))
done

echo "Points division by average:"
i=0
user_perc=0
for user in "${authors[@]}"; do
  user_perc=$(printf "%.0f" "$(bc -l <<<${points_avg[$i]}/$avg_points_sum*100)")

  echo " - ${user}: ${user_perc}%"
  ((i += 1))
done
