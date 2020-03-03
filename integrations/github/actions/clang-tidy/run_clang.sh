#!/bin/bash

cd $GITHUB_WORKSPACE

echo "clang arguments: $CLANG_ARGUMENTS"
echo "files modified: $FILES_MODIFIED"
echo "files added: $FILES_ADDED"

read -r -a MODIFIED_FILES <<< "$FILES_MODIFIED"
read -r -a ADDED_FILES <<< "$FILES_ADDED"

clang_arguments=""

for element in "${MODIFIED_FILES[@]}"
do
    clang_arguments+="$element "
done
for element in "${ADDED_FILES[@]}"
do
    clang_arguments+="$element "
done

clang_arguments+=" -export-fixes=fixes.yml"

clang_arguments+=" $CLANG_ARGUMENTS "


echo "final clang arguments: $clang_arguments"

# Run clang-tidy for real
clang-tidy $clang_arguments > clang_output.txt

if [ $? -eq 0 ]; then
    echo "Clang passed! Good job!"
    echo ::set-output name=clang-result::""
else
    CLANG_OUTPUT=`cat clang_output.txt`
    FIXES_OUTPUT=`cat fixes.yml`
    LINES_OF_FIXES=$(echo $FIXES_OUTPUT | wc -l)

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

    PULL_REQUEST_COMMENT_URL=$(cat $GITHUB_EVENT_PATH | jq -r .pull_request.comments_url)
      
    echo ::set-output name=clang-result::$CLANG_OUTPUT

    if [ ! -z "$PULL_REQUEST_COMMENT_URL" -a "$str"!="" -a "$str"!="null" ]; then
        echo "Posting to comment URL: $PULL_REQUEST_COMMENT_URL"

        REQUEST_DATA=$(echo '{}' | jq --arg body "$PULL_REQUEST_COMMENT" '.body = $body')
        curl -s -S -H "Authorization: token $GITHUB_TOKEN" --header "Content-Type: application/vnd.github.VERSION.text+json" --data "$REQUEST_DATA" "$PULL_REQUEST_COMMENT_URL"
    fi
fi
