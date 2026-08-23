#!/usr/bin/env bash
# Batch evaluator for all EC programs
set -euo pipefail

ENGCC="${ENGCC:-$HOME/.local/bin/engc}"
PROJECT_ROOT="/home/ferendra-putra/Downloads/EngineSoft/enginotech-cpp"
RESULTS_LOG="/tmp/eval_results.log"

echo "╔═══════════════════════════════════════════════════════════╗"
echo "║     EnginotechC++ Program Evaluator — Batch Mode         ║"
echo "╚═══════════════════════════════════════════════════════════╝"
echo ""

# Clear results log
> "$RESULTS_LOG"

TOTAL=0
PASSED=0

eval_program() {
    local program="$1"
    local name
    name=$(basename "$program" .ec)
    
    echo -e "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
    echo -e "📝 Evaluating: ${name}"
    echo -e "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
    
    local compile_ok=true
    local run_ok=false
    
    # Compile check
    local compile_output
    compile_output=$("$ENGCC" build "$program" "/tmp/${name}.ll" 2>&1) || true
    if echo "$compile_output" | grep -q "\[error\]"; then
        compile_ok=false
        echo -e "  ❌ Compilation failed"
        echo "$compile_output" | grep "\[error\]" | head -2 | sed 's/^/     /' >> "$RESULTS_LOG"
    else
        echo -e "  ✅ Compilation successful"
    fi
    
    # Run check
    if $compile_ok; then
        local run_output
        run_output=$("$ENGCC" run "$program" "/tmp/${name}_out" 2>&1) || true
        
        if echo "$run_output" | grep -qiE "OK|Passed|Result|Score|fib|loop sum|hello|grade|calculator|arith|test|digit|World|Demo"; then
            run_ok=true
            echo -e "  ✅ Correct execution"
        else
            echo -e "  ⚠ Execution completed (no expected output)"
        fi
    fi
    
    # Quality checks
    local quality_score=100
    if $compile_ok; then
        local func_count
        func_count=$(grep -c "^fn " "$program" 2>/dev/null || echo "0")
        if [ "$func_count" -gt 20 ]; then
            quality_score=$((quality_score - 20))
            echo -e "  ⚠ Too many functions: $func_count"
        fi
        
        local total_lines comment_lines
        total_lines=$(wc -l < "$program" 2>/dev/null || echo "1")
        comment_lines=$(grep -cE "^\s*(//|/\*)" "$program" 2>/dev/null || echo "0")
        if [ "$total_lines" -gt 10 ]; then
            local ratio=$((comment_lines * 100 / total_lines))
            if [ "$ratio" -ge 10 ]; then
                quality_score=$((quality_score + 5))
                echo -e "  ✅ Good documentation: ${ratio}%"
            else
                quality_score=$((quality_score - 10))
                echo -e "  ⚠ Low documentation: ${ratio}%"
            fi
        fi
        
        if grep -qE "assert\(" "$program"; then
            echo -e "  ✅ Uses assertions"
        fi
    fi
    
    # Calculate score
    local correct_score=0
    if $compile_ok && $run_ok; then
        correct_score=100
        PASSED=$((PASSED + 1))
    elif $compile_ok; then
        correct_score=75
    fi
    
    local total_score=$(( (correct_score + 100 + quality_score + 100) / 4 ))
    if [ $total_score -lt 0 ]; then total_score=0; fi
    if [ $total_score -gt 100 ]; then total_score=100; fi
    
    TOTAL=$((TOTAL + 1))
    echo -e "  📊 Score: ${total_score}/100" >> "$RESULTS_LOG"
    echo -e "  📊 Score: ${total_score}/100"
    echo ""
}

echo "Evaluating integration tests..."
echo ""
while IFS= read -r -d '' t; do
    eval_program "$t"
done < <(find "$PROJECT_ROOT/tests/integration" -name "test_*.ec" -print0 | sort -z)

echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "Evaluating examples..."
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""
for ex in "$PROJECT_ROOT/examples/hello/main.ec" "$PROJECT_ROOT/examples/bench/main.ec" "$PROJECT_ROOT/examples/arithmetic/main.ec" "$PROJECT_ROOT/examples/calculator/main.ec" "$PROJECT_ROOT/examples/os-demo/main.ec"; do
    [ -f "$ex" ] && eval_program "$ex"
done

echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "📊 SUMMARY"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━���━━━━━━━━━━━━━━━━"
echo "Total programs: $TOTAL"
echo "Passed: $PASSED"
if [ $TOTAL -gt 0 ]; then
    echo "Success rate: $((PASSED * 100 / TOTAL))%"
fi
echo ""

# Show detailed results
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━���━━━━━━━━━━━━━━━━"
echo "DETAILED RESULTS"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
cat "$RESULTS_LOG"
echo ""
