import * as React from "react"
import { render } from "react-dom"
import styled, { css } from "styled-components"

import { ComputerCard } from "./components/ComputerCard"
import { useState } from "react"
import { useEffect } from "react"
import { LoadingIndicator } from "./components/LoadingIndicator"

window.receiver = null
window.receive = function (message) {
  if (receiver) receiver(message)
}

const App = ({}) => {
  const [state, setState] = useState({
    phase: "initial",
  })

  useEffect(() => {
    window.receiver = setState
    return () => (window.receiver = null)
  })

  const handleEvent = (event) => {
    console.log(JSON.stringify({ type: "message", message: event }))
  }

  console.log(JSON.stringify(state))

  return (
    <AppLayout {...state}>
      {state.phase !== "running" && (
        <CentralMessage>
          {state.phase === "initial"
            ? "Conectando con el servidor central"
            : state.phase === "named"
            ? "Esperando otros clientes"
            : ""}
        </CentralMessage>
      )}
      {state.phase === "running" &&
        state.computers.map((c) => (
          <ComputerCard key={c.name} {...c} onEvent={handleEvent} />
        ))}
    </AppLayout>
  )
}

const cssForPhase = {
  initial: css`
    align-items: center;
    justify-content: center;
    height: 100vh;
  `,
  running: css`
    margin: 4px;
    flex-wrap: wrap;

    & > * {
      margin: 12px;
    }
  `,
}

const AppLayout = styled.div`
  display: flex;

  ${({ phase }) =>
    phase === "running" ? cssForPhase[phase] : cssForPhase["initial"]}
`

const CentralMessageLayout = styled.section`
  display: flex;
  flex-direction: column;
  align-items: center;
  user-select: none;

  font-size: 24px;
  text-align: center;
  color: rgba(0, 0, 0, 0.7);
`

const CentralMessage = ({ children }) => {
  return (
    <CentralMessageLayout>
      {children}
      <LoadingIndicator />
    </CentralMessageLayout>
  )
}

render(<App />, document.getElementById("app"))
