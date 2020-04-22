#!/usr/bin/env bash

cd "$GITHUB_WORKSPACE" || exit

echo "clang arguments: $CLANG_ARGUMENTS"
echo "files modified: $FILES_MODIFIED"
echo "files added: $FILES_ADDED"

# convert to arrays
read -r -a FILES_MODIFIED <<<"$FILES_MODIFIED"
read -r -a FILES_ADDED <<<"$FILES_ADDED"

# shellcheck disable=SC2206
CLANG_TIDY_ARGUMENTS=("${FILES_MODIFIED[@]}" "${FILES_ADDED[@]}" "$CLANG_TIDY_ARGUMENTS")

# nun clang-tidy
if clang-tidy "${CLANG_TIDY_ARGUMENTS[@]}" -export-fixes=fixes.yml >clang_output.txt; then
  echo "Clang passed! Good job!"
  echo ::set-output name=clang-result::""
else
  CLANG_OUTPUT=$(cat clang_output.txt)
  FIXES_OUTPUT=$(cat fixes.yml)

  # Cleanup the files we don't need
  rm clang_output.txt
  rm fixes.yml

  PULL_REQUEST_COMMENT=""
  PULL_REQUEST_COMMENT+=$'### Automated clang-tidy Review\n'

  PULL_REQUEST_COMMENT+=$'#### Clang Warnings & Errors\n'
  PULL_REQUEST_COMMENT+=$'\n```\n'
  PULL_REQUEST_COMMENT+="$CLANG_OUTPUT"
  PULL_REQUEST_COMMENT+=$'\n```\n'

  PULL_REQUEST_COMMENT+=$'#### Recommended Fixes\n'
  PULL_REQUEST_COMMENT+=$'\n```\n'
  PULL_REQUEST_COMMENT+="$FIXES_OUTPUT"
  PULL_REQUEST_COMMENT+=$'\n```\n'

  PULL_REQUEST_COMMENT_URL=$(jq -r .pull_request.comments_url <"$GITHUB_EVENT_PATH")

  echo ::set-output name=clang-result::"$CLANG_OUTPUT"

  if [[ -n $PULL_REQUEST_COMMENT_URL && ${str:?null} != null ]]; then
    echo "Posting to comment URL: $PULL_REQUEST_COMMENT_URL"

    REQUEST_DATA=$(echo '{}' | jq --arg body "$PULL_REQUEST_COMMENT" '.body = $body')
    curl -s -S -H "Authorization: token $GITHUB_TOKEN" --header "Content-Type: application/vnd.github.VERSION.text+json" --data "$REQUEST_DATA" "$PULL_REQUEST_COMMENT_URL"
  fi
fi
