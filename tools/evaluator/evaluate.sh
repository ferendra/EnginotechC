#!/usr/bin/env bash
# EC Program Evaluator — Assessment framework for EnginotechC++
# Evaluates: correctness, performance, code quality, best practices

set -euo pipefail
cd "$(dirname "${BASH_SOURCE[0]}")/.."

ENGCC="${ENGCC:-$HOME/.local/bin/engc}"
SCORES_FILE="evaluator_results.json"

# ─── Color codes ───
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

# ─── Score tracking ───
declare -A SCORES
TOTAL_TESTS=0
PASSED_TESTS=0
WARN_COUNT=0
ERROR_COUNT=0

# ═══════════════════════════════════════════════════════════
# SECTION 1: Correctness Evaluation
# ═══════════════════════════════════════════════════════════
evaluate_correctness() {
    local program="$1"
    local name=$(basename "$program" .ec)
    
    echo -e "${YELLOW}[1/4] Correctness${NC}"
    
    # Compile check
    local compile_output
    compile_output=$("$ENGCC" build "$program" "/tmp/${name}.ll" 2>&1) || true
    
    if echo "$compile_output" | grep -q "\[error\]"; then
        ERROR_COUNT=$((ERROR_COUNT + 1))
        echo -e "  ${RED}✗ Compilation failed${NC}"
        echo "$compile_output" | grep "\[error\]" | head -3 | sed 's/^/    /'
        SCORES["${name}_correctness"]=0
        return 1
    fi
    
    # Run check
    local run_output
    run_output=$("$ENGCC" run "$program" "/tmp/${name}_out" 2>&1) || true
    
    # Check for expected output patterns
    local has_output=false
    if echo "$run_output" | grep -qiE "OK|Passed|Result|Score|fib|loop sum|hello|grade|calculator|arith|test|digit"; then
        has_output=true
        PASSED_TESTS=$((PASSED_TESTS + 1))
    fi
    
    if $has_output; then
        echo -e "  ${GREEN}✓ Correct execution${NC}"
        SCORES["${name}_correctness"]=100
    else
        echo -e "  ${YELLOW}△ Execution completed (no expected output detected)${NC}"
        SCORES["${name}_correctness"]=50
    fi
    
    return 0
}

# ═══════════════════════════════════════════════════════════
# SECTION 2: Performance Benchmarking
# ═══════════════════════════════════════════════════════════
evaluate_performance() {
    local program="$1"
    local name=$(basename "$program" .ec)
    
    echo -e "${YELLOW}[2/4] Performance${NC}"
    
    # Measure execution time
    local start_time end_time elapsed
    start_time=$(date +%s.%N)
    "$ENGCC" run "$program" "/tmp/${name}_bench" >/dev/null 2>&1 || true
    end_time=$(date +%s.%N)
    elapsed=$(echo "$end_time - $start_time" | bc 2>/dev/null || echo "0.1")
    
    # Benchmark scoring (arbitrary scale)
    local score=100
    if (( $(echo "$elapsed > 1.0" | bc -l) )); then
        score=60
    elif (( $(echo "$elapsed > 0.5" | bc -l) )); then
        score=80
    fi
    
    echo "  Execution time: ${elapsed}s (score: ${score}/100)"
    SCORES["${name}_performance"]=$score
}

# ═══════════════════════════════════════════════════════════
# SECTION 3: Code Quality Analysis
# ═══════════════════════════════════════════════════════════
evaluate_code_quality() {
    local program="$1"
    local name=$(basename "$program" .ec)
    
    echo -e "${YELLOW}[3/4] Code Quality${NC}"
    
    local issues=0
    local warnings=0
    
    # Check for unused variables (heuristic)
    if grep -qE "let [a-z]+ = [^;]+" "$program" 2>/dev/null; then
        # Simple heuristic: if variable declared but not in subsequent lines
        local unused_vars=""
        while IFS= read -r var; do
            [ -z "$var" ] && continue
            local count
            count=$(grep -c "$var" "$program" 2>/dev/null || echo "0")
            if [ "$count" -eq 1 ]; then
                unused_vars="$unused_vars $var"
            fi
        done < <(grep -oE "let [a-z_]+ =" "$program" 2>/dev/null | awk '{print $2}')
        
        if [ -n "$unused_vars" ]; then
            warnings=$((warnings + 1))
            echo -e "  ${YELLOW}⚠ Unused variable(s):$unused_vars${NC}"
        fi
    fi
    
    # Check function complexity (simple heuristic)
    local func_count
    func_count=$(grep -c "^fn " "$program" 2>/dev/null || echo "0")
    
    if [ "$func_count" -gt 20 ]; then
        issues=$((issues + 1))
        echo -e "  ${RED}✗ Too many functions ($func_count) — consider refactoring${NC}"
    fi
    
    # Check for magic numbers
    local magic_count
    magic_count=$(grep -oE "[0-9]{2,}" "$program" 2>/dev/null | wc -l)
    if [ "$magic_count" -gt 10 ]; then
        warnings=$((warnings + 1))
        echo -e "  ${YELLOW}⚠ Many magic numbers detected ($magic_count)${NC}"
    fi
    
    SCORES["${name}_quality"]=$((100 - (issues * 20) - (warnings * 10)))
    if [ ${SCORES["${name}_quality"]} -lt 0 ]; then
        SCORES["${name}_quality"]=0
    fi
}

# ═══════════════════════════════════════════════════════════
# SECTION 4: Best Practices Check
# ═══════════════════════════════════════════════════════════
evaluate_best_practices() {
    local program="$1"
    local name=$(basename "$program" .ec)
    
    echo -e "${YELLOW}[4/4] Best Practices${NC}"
    
    local score=100
    
    # Check for main function
    if ! grep -q "^fn main()" "$program" 2>/dev/null && ! grep -q "fn main()" "$program" 2>/dev/null; then
        score=$((score - 20))
        echo -e "  ${YELLOW}⚠ No main() function found${NC}"
    fi
    
    # Check for return types
    local no_return_type
    no_return_type=$(grep -E "^fn [a-z_]+\(" "$program" | grep -v "->" | wc -l)
    if [ "$no_return_type" -gt 0 ] && ! grep -q "^fn main()" "$program"; then
        score=$((score - 10 * no_return_type))
        echo -e "  ${YELLOW}⚠ $no_return_type function(s) missing return type${NC}"
    fi
    
    # Check for comments
    local comment_ratio
    local total_lines
    total_lines=$(wc -l < "$program" 2>/dev/null || echo 1)
    local comment_lines
    comment_lines=$(grep -cE "^\s*(//|/\*)" "$program" 2>/dev/null || echo 0)
    if [ "$total_lines" -gt 10 ]; then
        local ratio=$((comment_lines * 100 / total_lines))
        if [ "$ratio" -lt 10 ]; then
            score=$((score - 10))
            echo -e "  ${YELLOW}⚠ Low comment ratio (${ratio}%)${NC}"
        fi
    fi
    
    # Check for error handling
    if grep -qE "assert\(" "$program"; then
        score=$((score + 10))  # Bonus for using assertions
        echo -e "  ${GREEN}✓ Uses assertions for validation${NC}"
    fi
    
    SCORES["${name}_best_practices"]=$score
}

# ═══════════════════════════════════════════════════════════
# MAIN EVALUATION LOOP
# ═══════════════════════════════════════════════════════════
evaluate_program() {
    local program="$1"
    
    echo ""
    echo "═══════════════════════════════════════════════════════════"
    echo "Evaluating: $(basename "$program")"
    echo "═══════════════════════════════════════════════════════════"
    
    evaluate_correctness "$program"
    evaluate_performance "$program"
    evaluate_code_quality "$program"
    evaluate_best_practices "$program"
    
    # Calculate total score
    local total=0
    local count=0
    for key in "${!SCORES[@]}"; do
        if [[ "$key" == "$(basename "$program" .ec)_"* ]]; then
            total=$((total + SCORES[$key]))
            count=$((count + 1))
        fi
    done
    
    if [ $count -gt 0 ]; then
        local avg=$((total / count))
        echo ""
        echo -e "  ${GREEN}Overall Score: $avg/100${NC}"
        echo "    Correctness: ${SCORES["$(basename "$program" .ec)_correctness"]}/100"
        echo "    Performance: ${SCORES["$(basename "$program" .ec)_performance"]}/100"
        echo "    Quality: ${SCORES["$(basename "$program" .ec)_quality"]}/100"
        echo "    Best Practices: ${SCORES["$(basename "$program" .ec)_best_practices"]}/100"
    fi
}

# ─── Entry point ───
if [ $# -eq 0 ]; then
    echo "EC Program Evaluator"
    echo "Usage: ./tools/evaluator/evaluate.sh <program.ec>"
    echo ""
    echo "Examples:"
    echo "  ./tools/evaluator/evaluate.sh examples/hello/main.ec"
    echo "  ./tools/evaluator/evaluate.sh tests/integration/test_arith.ec"
    exit 0
fi

evaluate_program "$1"
