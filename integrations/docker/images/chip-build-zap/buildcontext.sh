repo_root=$(dirname $(readlink -e '../../../'))
echo $(git --git-dir=$repo_root/.git show -s --format=%H) > context.txt
