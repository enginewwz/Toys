-- 1) 代码块 -> minted（行号/底色/边框在导言区 \setminted 里统一设置）
--    Pygments 没有 gdb lexer，直接交给 minted 会报错，这里回退到 text。
local LANG_ALIAS = {
  gdb = "text",
}

function CodeBlock(cb)
  local lang = "text"
  if #cb.classes > 0 then lang = cb.classes[1] end
  lang = LANG_ALIAS[lang] or lang
  return pandoc.RawBlock("latex",
    "\\begin{minted}{" .. lang .. "}\n" .. cb.text .. "\n\\end{minted}")
end

-- 2) 正文里的 ASCII 双引号 -> 中文弯引号
--    pandoc 的 smart 对 CJK 会全部判成右引号，所以用 -smart 关掉它、在这里自行配对。
--    只处理 Str（正文），行内代码与代码块不受影响。
local OPEN  = "\xe2\x80\x9c"   -- U+201C
local CLOSE = "\xe2\x80\x9d"   -- U+201D
local qopen = true
function Str(el)
  if not el.text:find('"', 1, true) then return nil end
  local out, buf = {}, {}
  for _, cp in utf8.codes(el.text) do
    if cp == 0x22 then
      if #buf > 0 then out[#out + 1] = pandoc.Str(table.concat(buf)); buf = {} end
      out[#out + 1] = pandoc.RawInline("latex", qopen and OPEN or CLOSE)
      qopen = not qopen
    else
      buf[#buf + 1] = utf8.char(cp)
    end
  end
  if #buf > 0 then out[#out + 1] = pandoc.Str(table.concat(buf)) end
  return out
end
