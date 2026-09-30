#!/usr/bin/env bash
# report.md  ->  report.tex  ->  report.pdf
#
# 用法：  bash .texbuild/build.sh
#
# 依赖：  pandoc (>= 3)           markdown -> latex 片段
#        xelatex (TeX Live)       需 -shell-escape（minted 要调 pygmentize）
#        pygmentize (Pygments)    代码高亮
#
# 代码块样式在 .texbuild/preamble.tex 的 \setminted 里统一设置：
#   极浅灰衬底 black!3、深灰细边框 black!60、左侧行号、超长行自动折行。
# 正文里的 ASCII 双引号由 .texbuild/filter.lua 配对成中文弯引号 “ ”，
#   同时 pandoc 用 -smart 关掉自带的 smart quotes（它对 CJK 会全判成右引号）。
#
# 产物布局：只有 report.tex / report.pdf 落在仓库根，其余全部关进 .texbuild/out/
#   （.aux/.log/_minted/ 等中间产物），编译在子目录内进行以免污染根目录。
set -euo pipefail

cd "$(dirname "$0")/.."                      # -> CA_hw/Sept30
TMP=$(mktemp -d); trap 'rm -rf "$TMP"' EXIT

TITLE=$(sed -n '1s/^# //p' report.md)
AUTHOR=$(sed -n '3s/^> //p' report.md)

# 导言区：填入标题，以及标题下方的"姓名 + 学号"一行
sed -e "s|@@TITLE@@|$TITLE|" -e "s|@@AUTHOR@@|$AUTHOR|" \
    .texbuild/preamble.tex > "$TMP/preamble.tex"

# 正文：丢掉标题行、姓名行和紧随其后的第一条分隔线，标题层级整体上提一级
# （要整体上提一级，所以用一次替换删掉一个 #，避免链式 sed 连降两级）
awk 'BEGIN{skip=1} skip && /^---$/{skip=0; next} skip{next} {print}' report.md \
  | sed -E 's/^#(#{1,5}) /\1 /' > "$TMP/body.md"

pandoc "$TMP/body.md" -f markdown-smart -t latex \
  --lua-filter=.texbuild/filter.lua -o "$TMP/body.tex"

# 章节号已经写在标题文字里，改用不带编号的 \section*
perl -0pi -e 's/^\\subsubsection\{/\\subsubsection*{/gm; s/^\\subsection\{/\\subsection*{/gm; s/^\\section\{/\\section*{/gm' "$TMP/body.tex"

cat "$TMP/preamble.tex" "$TMP/body.tex" > report.tex
printf '\n\\end{document}\n' >> report.tex

# 正文没有外部文件依赖，因此把 report.tex 复制进子目录、在那里编译，
# .aux/.log/_minted/ 等中间产物就都留在 .texbuild/out/ 里，根目录保持干净。
OUT=.texbuild/out
rm -rf "$OUT"; mkdir -p "$OUT"
cp report.tex "$OUT/report.tex"

# minted 要把高亮结果写进 .pygtex，跑三遍确保行号与内容都稳定
for _ in 1 2 3; do
  if ! (cd "$OUT" && xelatex -shell-escape -interaction=nonstopmode \
          -file-line-error report.tex) > .texbuild/build.log 2>&1; then
    echo "xelatex 失败，见 .texbuild/build.log" >&2
    grep -nE '^!' .texbuild/build.log | head >&2
    exit 1
  fi
done

cp "$OUT/report.pdf" report.pdf

grep -o 'Output written on report.pdf ([0-9]* pages' .texbuild/build.log || true
echo "OK: report.tex / report.pdf 已更新（中间产物在 .texbuild/out/）"
