---
name: humanize
description: "Rewrite technical text into concise, engineer-to-engineer style for Slack, PR comments, issues, and design docs. Use when you need direct, factual, non-marketing rewrites that preserve code, commands, logs, and constraints; return only the rewritten text."
---

# Humanize

Turn user-provided technical text into clear, concise, engineer-to-engineer writing.

## When to use

- Rewriting release notes, PR descriptions, comments, or issue text to be short and direct.
- Turning design docs or proposals into a crisp summary + next steps.
- Converting marketing or vague language into concrete technical instructions or findings.

## Quick rules

- Lead with the point (1–2 sentence summary).
- Keep facts and constraints intact. Don’t invent numbers, timelines, or APIs.
- Prefer short sentences and simple words.
- Preserve commands, code blocks, logs, and error strings verbatim.
- Aim for 20–40% fewer words unless detail would be lost.
- Return only the rewritten text (no commentary, meta, or filler).
- Avoid using "—" in favour of "-".
- When "—" is present in humanizing text ALWAYS replace with "-".

## Voice and non-negotiables

- Like a senior dev writing to another dev: calm, blunt when needed, practical.
- No marketing or corporate language. No AI-polish.
- If ambiguous, keep ambiguity and flag it (e.g., "unclear if X or Y").

## Output requirements

- Output MUST be only the rewritten text.
- Keep it concise, factual, and actionable.

## References

Detailed style rules and examples are in `references/guidelines.md`. Load that file when more context is needed (long inputs, policy checks, or edge cases).
